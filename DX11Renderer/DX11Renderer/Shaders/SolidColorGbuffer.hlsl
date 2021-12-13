#include "PhongCommon.hlsli"

cbuffer LightCBuf : register(b0)
{
    float3 lightPos;
    float lightInvRangeSqr;
    float3 lightColor;
    float lightIntensity;
};

cbuffer ObjectCBuf : register(b1)
{
    float3 materialColor;
    float roughness;
    bool normalMapEnabled; // 4 bytes in HLSL, so use BOOL in C++ to match
    float specularPower;
    float padding[2];
};

/*cbuffer CBuf : register(b2)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};*/

struct PS_OUTPUT
{
    float4 NormalRough : SV_Target0;
    float4 Second : SV_Target1;
};

PS_OUTPUT main(v2f i) : SV_Target
{
    PS_OUTPUT o;
    o.NormalRough = 0;
    o.Second = 0;
    return o;
}