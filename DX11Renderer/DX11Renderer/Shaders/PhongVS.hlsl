
#include "PhongCommon.hlsli"

cbuffer CBuf : register(b0)
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
};

struct attrib
{
    float3 pos : Position;
    float3 n : Normal;
    float3 t : Tangent;
    float2 uv0 : TEXCOORD0;
};

v2f main(attrib i)
{
    v2f vso;
    vso.positionVS = (float3) mul(modelView, float4(i.pos, 1.0f));
    vso.normalWS = mul((float3x3) model, i.n);;
    vso.normalVS = mul((float3x3) modelView, i.n);
    vso.tangentVS = mul((float3x3) modelView, i.t);
	vso.pos = mul(modelViewProj, float4(i.pos, 1.0f));
    vso.uv0 = i.uv0;
	return vso;
}