#include "CbufCommon.hlsli"

struct attrib
{
	float3 pos : Position;
	float3 n : Normal;
	float3 t : Tangent;
	float2 uv0 : Texcoord0;
	float4x4 instanceTransform : INSTANCE_TRANSFORM;
	float4 instanceColor : INSTANCE_COLOR;
};

cbuffer PerObjectTransformCB : register(b3)
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
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