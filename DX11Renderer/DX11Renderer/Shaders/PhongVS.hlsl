
#include "CbufCommon.hlsli"
#include "PhongCommon.hlsli"

cbuffer CBuf : register(b2)
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
    float2 uv0 : Texcoord0;
    float3 instancePosition : INSTANCEPOS;
};

float4 ComputeNonStereoScreenPos(float4 pos)
{
    float4 o = pos * 0.5f;
    o.xy = float2(o.x, o.y * _ProjectionParams.x) + o.w;
    o.zw = pos.zw;
    return o;
}

v2f main(attrib i)
{
    v2f vso;
    i.pos -= i.instancePosition * 0.5;
    vso.positionVS = (float3) mul(modelView, float4(i.pos, 1.0f));
    vso.normalWS = mul((float3x3) model, i.n);;
    vso.normalVS = mul((float3x3) modelView, i.n);
    vso.tangentVS = mul((float3x3) modelView, i.t);
	vso.pos = mul(modelViewProj, float4(i.pos, 1.0f));
    vso.uv0 = float2(i.uv0.x, 1.f - i.uv0.y);
	return vso;
}