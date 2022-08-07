
#include "CbufCommon.hlsli"

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

struct v2f
{
    float4 pos : SV_POSITION;
    float depth : TEXCOORD0;
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
    o.pos = mul(_ViewProjMatrix, mul(i.instanceTransform, float4(i.pos, 1.0f)));
#else
    o.pos = mul(modelViewProj, float4(i.pos, 1.0f));
#endif
    o.depth = o.pos.z;
    return o;
}