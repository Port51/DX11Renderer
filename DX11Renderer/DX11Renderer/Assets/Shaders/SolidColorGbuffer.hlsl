#include "PhongCommon.hlsli"
#include "./CbufCommon.hlsli"

cbuffer LightCBuf : register(b0)
{
    float3 lightPos;
    float lightInvRangeSqr;
    float3 lightColor;
    float lightIntensity;
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