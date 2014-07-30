// Constant buffers registered to slots to store
// the worldMat, viewMat, projMat
cbuffer cbPerObject : register( b0 )
{
	float4x4 g_world;
};

cbuffer cbNeverChanges : register( b1 )
{
	float4x4 g_view;
};

cbuffer cbChangeOnResize : register( b2 )
{
	float4x4 g_proj;
};

//Describes the VertexIn type for the vertex shader
struct VertexIn
{
	float4 Pos  : POSITION;
    float4 Color : COLOR;
};

//Describes the VertexOut type for the pixel shader
struct VertexOut
{
	float4 Pos  : SV_POSITION;
    float4 Color : COLOR;
};

//The vertex shader main entry
VertexOut VS_Main(VertexIn vin)
{
	VertexOut vout;

	//Transform the position of vertex by the world, view and projection matrix;
	vout.Pos = mul(vin.Pos, g_world);
	vout.Pos = mul(vout.Pos, g_view);
	vout.Pos = mul(vout.Pos, g_proj);

	//Assign the vertex color;
    vout.Color = vin.Color;

    return vout;
}

//The pixel shader main entry
float4 PS_Main(VertexOut pin) : SV_Target
{
    return pin.Color;
}
