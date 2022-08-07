#include "./CbufCommon.hlsli"
#include "./GerstnerWaves.hlsli"

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

	float3 positionWS = (float3) mul(model, float4(i.pos, 1.0f)).xyz;
	positionWS = GetGerstnerWaves(positionWS);

	o.pos = mul(_ViewProjMatrix, float4(positionWS, 1.0f));
	//o.pos = mul(modelViewProj, float4(i.pos, 1.0f));
	return o;
}