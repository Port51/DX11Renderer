
#include "CbufCommon.hlsli"
#include "PhongCommon.hlsli"

struct attrib
{
    float3 pos : Position;
    float3 n : Normal;
    float3 t : Tangent;
    float4 vertColor : VERTCOLOR0;
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

float4 ComputeNonStereoScreenPos(float4 pos)
{
    float4 o = pos * 0.5f;
    o.xy = float2(o.x, o.y * _ProjectionParams.x) + o.w;
    o.zw = pos.zw;
    return o;
}

v2f main(attrib i)
{
    v2f o;

#if defined(INSTANCING_ON)
    o.positionWS = (float3) mul(i.instanceTransform, float4(i.pos, 1.0f)).xyz;
    o.positionVS = (float3) mul(_ViewMatrix, float4(o.positionWS.xyz, 1.0f)).xyz;
    o.normalWS = mul((float3x3) i.instanceTransform, i.n).xyz;
    o.normalVS = mul((float3x3) _ViewMatrix, o.normalWS).xyz;
    o.tangentVS = mul((float3x3)_ViewMatrix, mul((float3x3)i.instanceTransform, i.t)).xyz;
    o.pos = mul(_ViewProjMatrix, float4(o.positionWS, 1.0f));
#else
    o.positionWS = (float3) mul(model, float4(i.pos, 1.0f)).xyz;
    o.positionVS = (float3) mul(modelView, float4(i.pos, 1.0f)).xyz;
    o.normalWS = mul((float3x3) model, i.n).xyz;
    o.normalVS = mul((float3x3) modelView, i.n).xyz;
    o.tangentVS = mul((float3x3) modelView, i.t).xyz;
    o.pos = mul(modelViewProj, float4(i.pos, 1.0f));
#endif
    o.positionNDC = o.pos;
    o.uv0 = float2(i.uv0.x, 1.f - i.uv0.y);
    o.vertColor = i.vertColor;
    o.screenPos = o.pos;
	return o;
}