#include "DxGraphics.h"

DxGraphics::DxGraphics() : m_windowWidth(0), m_windowHeight(0), 
	m_enableMsaa(true), m_windowed(true), m_shutdown(false),
	m_msaaQuality(0), m_sampleCount(4), m_featureLevel(D3D_FEATURE_LEVEL_11_0),
	m_device(0), m_immediateContext(0), m_swapChain(0), m_depthStencilView(0),
	m_renderTargetView(0), m_depthStencilBuffer(0), m_enableWireframe(false),
	m_wireframe(0)
{
}

DxGraphics::~DxGraphics()
{
	if(!m_shutdown)
	{
		Shutdown();
	}
}

bool DxGraphics::InitialiseDX(HWND hwnd, int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	HRESULT hr;
	UINT creationFlags = 0;

	//If compiler is run in debug mode then create device in debug mode
#if defined (DEBUG) | defined (_DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//Create the D3D device

	hr = D3D11CreateDevice(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		creationFlags,
		0, 0,
		D3D11_SDK_VERSION,
		&m_device,
		&m_featureLevel,
		&m_immediateContext);

	//If the device returned was not using directX11 feature level then return false
	if(m_featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBoxA(NULL, "D3D11 not supported on this device", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	//If the device failed to create then return false
	if(FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to create the D3D device", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	//Check the GPU'S 4xmsaa quality levels
	HR(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_sampleCount, &m_msaaQuality));

	//Throw an asserstion error if the 4xmsaaQuality is less then 0
	if(m_msaaQuality < 0)
	{
		MessageBoxA(NULL, "Cannot suppport this MSAA sample level", "ERROR!", MB_OK | MB_ICONERROR);
		m_enableMsaa = false;
	}

	//Describe the swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	scd.BufferDesc.Width = m_windowWidth;
	scd.BufferDesc.Height = m_windowHeight;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//If 4xmsaa is enabled then use the gpu 4xmsaa quality
	if(m_enableMsaa)
	{
		scd.SampleDesc.Count = m_sampleCount;
		scd.SampleDesc.Quality = m_msaaQuality-1;
	}
	else
	{
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
	}

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = m_windowed;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Find the factory used creating the device
	IDXGIDevice* dxgiDevice = 0;
	HR(m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	//Create the swap chain using the factory used when creating the device
	HR(dxgiFactory->CreateSwapChain(m_device, &scd, &m_swapChain));

	//Release all the COM objects used to create the swapchain
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	//Create the rasterizer for no backface culling
	D3D11_RASTERIZER_DESC noBackFaceDesc;
	ZeroMemory(&noBackFaceDesc, sizeof(D3D11_RASTERIZER_DESC));
	noBackFaceDesc.FillMode = D3D11_FILL_SOLID;
	noBackFaceDesc.CullMode = D3D11_CULL_NONE;

	HR(m_device->CreateRasterizerState(&noBackFaceDesc, &m_noBackFaceCulling));

	m_immediateContext->RSSetState(m_noBackFaceCulling);

	//Create the rasterizer state for wireframe mode
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_NONE;
	/*wireframeDesc.FillMode = D3D11_FILL_SOLID;
	wireframeDesc.CullMode = D3D11_CULL_BACK;*/

	HR(m_device->CreateRasterizerState(&wireframeDesc, &m_wireframe));

	//Rebuild the SwapChain and render target views
	if(!Rebuild(hwnd))
	{
		return false;
	}

	return true;
}

bool DxGraphics::Rebuild(HWND hwnd)
{

	//Assertion error if the device and immediate context have not been created
	assert(m_device);
	assert(m_immediateContext);
	assert(m_swapChain);

	//Release old target views
	ReleaseCOM(m_renderTargetView);
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_depthStencilBuffer);

	//Resize the swapchains buffers
	HR(m_swapChain->ResizeBuffers(1, m_windowWidth, m_windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	//Create the render target view
	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_device->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	//Describe the depth stencil view
	D3D11_TEXTURE2D_DESC dsd;
	dsd.Width = m_windowWidth;
	dsd.Height = m_windowHeight;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	if(m_enableMsaa)
	{
		dsd.SampleDesc.Count = m_sampleCount;
		dsd.SampleDesc.Quality = m_msaaQuality -1;
	}
	else
	{
		dsd.SampleDesc.Count = 1;
		dsd.SampleDesc.Quality = 0;
	}

	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsd.MiscFlags = 0;
	dsd.CPUAccessFlags = 0;

	//Create the depth stencil buffer
	HR(m_device->CreateTexture2D(&dsd, 0, &m_depthStencilBuffer));
	
	//Create the depth stencil view
	HR(m_device->CreateDepthStencilView(m_depthStencilBuffer, 0, &m_depthStencilView));

	//Bind the render target views and the depth stencil view to the Output Merger stage
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	//Set up the viewport
	m_viewport.Width = static_cast<float>(m_windowWidth);
	m_viewport.Height = static_cast<float>(m_windowHeight);
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	m_immediateContext->RSSetViewports(1, &m_viewport);

	return true;
}

void DxGraphics::StartRender()
{
	//Clear the render target view back to a blue background
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, 
		reinterpret_cast<const float*>(&XMFLOAT4(0.1f, 0.1f, 0.5f, 0.5f)));

	//Clear the depthStencil view to a depth of 1.0f
	m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//Sets to wireframe mode if enabled
	if(m_enableWireframe)
	{
		m_immediateContext->RSSetState(m_wireframe);
	}
	else
	{
		m_immediateContext->RSSetState(m_noBackFaceCulling);
	}
}

void DxGraphics::EndRender()
{
	//Present the back buffer
	m_swapChain->Present(1, 0);
}

void DxGraphics::Shutdown()
{
	if(!m_shutdown)
	{
		//set the swap chain back to windowed for releasing
		m_swapChain->SetFullscreenState(FALSE, NULL);

		ReleaseCOM(m_device);
		ReleaseCOM(m_immediateContext);
		ReleaseCOM(m_swapChain);
		ReleaseCOM(m_depthStencilView);
		ReleaseCOM(m_renderTargetView);
		ReleaseCOM(m_depthStencilBuffer);
		ReleaseCOM(m_wireframe);
		ReleaseCOM(m_noBackFaceCulling);

		m_shutdown = true;
	}
}

ID3D11Device* DxGraphics::GetDevice()
{
	return m_device;
}
ID3D11DeviceContext* DxGraphics::GetImmediateContext()
{
	return m_immediateContext;
}
IDXGISwapChain* DxGraphics::GetSwapChain()
{
	return m_swapChain;
}
ID3D11DepthStencilView* DxGraphics::GetDepthStencilView()
{
	return m_depthStencilView;
}
ID3D11RenderTargetView* DxGraphics::GetRenderTargetView()
{
	return m_renderTargetView;
}
ID3D11Texture2D* DxGraphics::GetDepthStencilBuffer()
{
	return m_depthStencilBuffer;
}
bool DxGraphics::GetWireframeMode()
{
	return m_enableWireframe;
}

void DxGraphics::SetWireframeMode(bool wireframe)
{
	m_enableWireframe = wireframe;
}
void DxGraphics::SetWindowWidth(int windowWidth)
{
	m_windowWidth = windowWidth;
}
void DxGraphics::SetWindowHeight(int windowHeight)
{
	m_windowHeight = windowHeight;
}
void DxGraphics::SetWindowed(bool windowed)
{
	m_windowed = windowed;
}
