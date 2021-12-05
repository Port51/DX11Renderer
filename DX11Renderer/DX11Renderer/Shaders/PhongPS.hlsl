
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

Texture2D tex : register(t0);
Texture2D nmap : register(t1);
SamplerState splr : register(s0);

#include "Lighting/BRDF.hlsli"

float SCurve(float x)
{
    // (3x^2 - 2x^3)
    return (-2 * x + 3) * x * x; // OPS: [MAD] [MUL] [MUL]
}

float4 main(v2f i) : SV_Target
{
    return 1.f;
}