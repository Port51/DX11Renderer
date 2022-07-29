#include "CbufCommon.hlsli"

cbuffer CBuf : register(b3)
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
};

struct attrib
{
	float3 pos : Position;
};

struct v2f
{
	float4 pos : SV_POSITION;
};

v2f main(attrib i)
{
	v2f o;
	o.pos = mul(modelViewProj, float4(i.pos, 1.0f));
	return o;
}