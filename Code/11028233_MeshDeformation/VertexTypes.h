#ifndef VERTEXTYPES_H
#define VERTEXTYPES_H

struct VertexPos
{
	XMFLOAT3 position;
	XMFLOAT4 colour;
};

struct VertexPosNor
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 colour;
};

struct VertexPosTex
{
	XMFLOAT3 position;
	XMFLOAT2 texture;
};

struct VertexPosNorTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
};

#endif