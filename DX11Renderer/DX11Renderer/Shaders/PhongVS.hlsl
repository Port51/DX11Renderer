
#include "PhongCommon.hlsli"

cbuffer CBuf : register(b0)
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
};

v2f main(float3 pos : Position, float3 n : Normal, float2 uv0 : TEXCOORD0)
{
    v2f vso;
    vso.positionVS = (float3) mul(modelView, float4(pos, 1.0f));
    vso.normalWS = n;
    vso.normalVS = mul((float3x3) modelView, n);
	vso.pos = mul(modelViewProj, float4(pos, 1.0f));
    vso.uv0 = uv0;
	return vso;
}