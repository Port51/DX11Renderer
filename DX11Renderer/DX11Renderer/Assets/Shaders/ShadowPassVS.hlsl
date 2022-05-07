
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
    float3 n : Normal;
    float3 t : Tangent;
    float2 uv0 : Texcoord0;
    float3 instancePosition : INSTANCEPOS;
};

struct v2f
{
    float4 pos : SV_POSITION;
    float depth : TEXCOORD0;
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
    o.pos = mul(modelViewProj, float4(i.pos, 1.0f));
    o.depth = o.pos.z;
    return o;
}