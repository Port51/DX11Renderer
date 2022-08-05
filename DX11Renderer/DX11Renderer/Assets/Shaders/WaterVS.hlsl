
#include "./CbufCommon.hlsli"
#include "./GerstnerWaves.hlsli"
#include "./PhongCommon.hlsli"

struct attrib
{
    float3 pos : Position;
    float3 n : Normal;
    float3 t : Tangent;
    float2 uv0 : Texcoord0;
    float3 instancePosition : INSTANCEPOS;
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
    i.pos -= i.instancePosition * 0.5;

    float3 positionWS = (float3) mul(model, float4(i.pos, 1.0f)).xyz; 
    positionWS = GetGerstnerWaves(positionWS);

    o.positionVS = (float3) mul(_ViewMatrix, float4(positionWS, 1.0f)).xyz;
    o.positionWS = positionWS;

    float3 tangent = GetGerstnerWavesTangent(positionWS);
    float3 bitangent = GetGerstnerWavesBitangent(positionWS);
    float3 normal = cross(tangent, bitangent);

    o.normalWS = mul((float3x3) model, normal).xyz;
    o.normalVS = mul((float3x3) modelView, normal).xyz;
    o.tangentVS = mul((float3x3) modelView, tangent).xyz;
    o.pos = mul(_ViewProjMatrix, float4(positionWS, 1.0f));
    o.positionNDC = o.pos;
    o.uv0 = float2(i.uv0.x, 1.f - i.uv0.y);
    o.screenPos = o.pos;
    return o;
}