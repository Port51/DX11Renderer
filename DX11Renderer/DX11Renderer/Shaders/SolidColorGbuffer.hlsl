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

float4 main(v2f i) : SV_Target
{
    return 0;
}