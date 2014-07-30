/*
	Contains all D3d initialization and accessors to all the D3D COM objects
*/

#ifndef DXGRAPHICS_H
#define DXGRAPHICS_H

#include <Windows.h>
#include <assert.h>
#include <D3DX11.h>
#include <D3D11.h>
#include <DxErr.h>
#include <D3Dcompiler.h>
#include <D3Dcommon.h>
#include<xnamath.h>

#include "Utilities.h"

using namespace Utilities;

class DxGraphics
{
public:
	DxGraphics();
	~DxGraphics();

	bool InitialiseDX(HWND hwnd, int windowWidth, int windowHeight);
	void StartRender();
	void EndRender();
	void Shutdown();

	bool Rebuild(HWND hwnd);

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetImmediateContext();
	IDXGISwapChain* GetSwapChain();
	ID3D11DepthStencilView* GetDepthStencilView();
	ID3D11RenderTargetView* GetRenderTargetView();
	ID3D11Texture2D* GetDepthStencilBuffer();

	void SetWindowWidth(int windowWidth);
	void SetWindowHeight(int windowHeight);
	void SetWindowed(bool windowed);

	void SetWireframeMode(bool wireframe);

	bool GetWireframeMode();

private:
	int m_windowWidth,
		m_windowHeight;

	bool m_enableMsaa,
		m_windowed,
		m_shutdown,
		m_enableWireframe;

	UINT m_msaaQuality,
		m_sampleCount;

	D3D_FEATURE_LEVEL m_featureLevel;

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_immediateContext;
	IDXGISwapChain* m_swapChain;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11RasterizerState* m_wireframe;
	ID3D11RasterizerState* m_noBackFaceCulling;

	D3D11_VIEWPORT m_viewport;
};

#endif