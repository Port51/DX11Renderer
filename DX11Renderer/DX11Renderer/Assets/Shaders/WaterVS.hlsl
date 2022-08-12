
#include "./CbufCommon.hlsli"
#include "./GerstnerWaves.hlsli"
#include "./PhongCommon.hlsli"

struct attrib
{
    float3 pos : Position;
    float3 n : Normal;
    float3 t : Tangent;
    float2 uv0 : Texcoord0;
    float4x4 instanceTransform : INSTANCE_TRANSFORM;
    float4 instanceColor : INSTANCE_COLOR;
    float4 instanceRngAndIndex : INSTANCE_RNG_AND_INDEX;
};

cbuffer PerObjectTransformCB : register(b3)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

v2f main(attrib i)
{
    v2f o;

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
    o.uv0 = float2(i.uv0.x, i.uv0.y);
    return o;
}