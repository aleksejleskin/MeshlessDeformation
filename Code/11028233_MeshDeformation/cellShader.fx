uint NUM_OF_LIGHTS = 10;

// Constant buffers registered to slots to store
// the worldMat, viewMat, projMat
cbuffer cbPerObject : register(b0)
{
	float4x4 g_world;
};

cbuffer cbNeverChanges : register(b1)
{
	float4x4 g_view;
};

cbuffer cbChangeOnResize : register(b2)
{
	float4x4 g_proj;
};
//Constant buffer to store the light infomation
cbuffer cbLights : register(b3)
{
	float4 g_lightPos[10];
	float4 g_lightAmount;
};

//constant buffer to store the camera position
cbuffer cbCameraPos : register(b4)
{
	float4 g_camPos;
};

cbuffer cbInvWorld : register(b5)
{
	float4x4 g_invWorld;
}

//Describes the VertexIn type for the vertex shader
struct VertexIn
{
	float3 Pos  : POSITION;
	float3 Norm : NORMAL;
	float4 Colour : COLOR;
};

//Describes the VertexOut type for the pixel shader
struct VertexOut
{
	float4 Pos  : SV_POSITION;
	float3 Norm : NORMAL;
	float4 Colour : COLOR;
	float3 dir[10] : LIGHTDIRECTION;
	float edge : EDGE;
};

//The vertex shader main entry
VertexOut VS_Main(VertexIn vin)
{
	VertexOut vout;

	//Transform the position of vertex by the world, view and projection matrix;
	vout.Pos = mul(float4(vin.Pos, 1.0f), g_world);
	vout.Pos = mul(vout.Pos, g_view);
	vout.Pos = mul(vout.Pos, g_proj);

	//transform the normal into world space
	vout.Norm = normalize(mul(vin.Norm, (float3x3)g_invWorld));
	//vout.Norm = normalize(mul(vin.Norm, (float3x3)g_world));

	//Assign the vertex colour;
	vout.Colour = vin.Colour;

	//find the direction between the light source and the vertex for the diffuse part of the cel shader
	[unroll]
	for (uint i = 0; i < 10; i++)
	{
		float3 direction = (float3)normalize(g_lightPos[i] - mul((float4)(vin.Pos, 1.0f), g_world));
			vout.dir[i] = normalize(direction);
	}

	//WORKS OUT THE OUTLINE
	//transformed normal into projection space
	float4 edgeNorm = (mul((float4)(vin.Norm, 1.0f), g_world));
		edgeNorm = (mul(edgeNorm, g_view));
	edgeNorm = (mul(edgeNorm, g_proj));

	//Get the dot product of the camera to the vertex
	float3 targ = (float3)g_camPos - (float3)(mul((float4)(vin.Pos, 1.0f), g_world));
		//float4 targ = g_camPos - vout.Pos;
		float edgeDot = dot(targ, vout.Norm);

	//Find the magnitudes of both vectors to be used to find the angle of the dot product
	float targLength = length(targ);

	//Check to see if the camera isnt so close to the object that it distorts it
	if (targLength > 5.0f)
	{
		float normLength = length(vout.Norm);

		//find the angle of the dot product
		float multiMag = (targLength*normLength);
		float angle = acos(edgeDot / multiMag);
		//float angle = acos(edgeDot);

		//if the angle is greater then 90 degrees then the vertex is on the edge
		// extrude the vertex out and set it to be coloured in black in the pixel shader
		if (angle >= radians(75.0f))
		{
			//vout.Pos = vout.Pos + mul(0.5f, normalize(edgeNorm));
			//vout.Colour = float4(0,0,0,1);
			vout.edge = 1.0f;
		}
		else
		{
			vout.edge = 0.0f;
		}
	}
	else
	{
		vout.edge = 0.0f;
	}
	return vout;
}

//The pixel shader main entry
float4 PS_Main(VertexOut pin) : SV_Target
{
	//Set the colour of the pixel
	float4 colour = pin.Colour;

	float3 dir;
	float intensity = 0.0f;

	uint lightAmount = (uint)g_lightAmount.x;

	//Find the intensity of the diffuse light by using the dot product
	[unroll]
	for (uint i = 0; i < lightAmount; i++)
	{
		dir = pin.dir[i];
		intensity = intensity + float(saturate(dot(dir, pin.Norm)));
	}

	//Depending on its value diffuse the colour
	float diffuse = intensity;

	//if (intensity > 0.95)
	//	diffuse = 1.0f;
	//else if (intensity > 0.5)
	//	diffuse = 0.7f;
	//else if (intensity > 0.05)
	//	diffuse = 0.35f;
	//else
	//	diffuse = 0.1f;

	if (pin.edge > 0.8f)
	{
		colour = float4(0, 0, 0, 1);
	}

	float4 outColour = colour*diffuse;
		return outColour;
}
