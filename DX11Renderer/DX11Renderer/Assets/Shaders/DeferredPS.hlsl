
#include "./CbufCommon.hlsli"
#include "./PhongCommon.hlsli"

cbuffer LightCBuf : register(b0)
{
	float3 lightPos;
    float lightInvRangeSqr;
	float3 lightColor;
	float lightIntensity;
};

cbuffer ObjectCBuf : register(b5)
{
    float3 materialColor;
    float roughness;
    bool normalMapEnabled; // 4 bytes in HLSL, so use BOOL in C++ to match
    float specularPower;
    float reflectivity;
    float padding[1];
};

/*cbuffer CBuf : register(b2)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};*/

// Per-frame
Texture2D SpecularLightingRT : register(t5);
Texture2D DiffuseLightingRT : register(t6);

// Per-draw
Texture2D tex : register(t0);
Texture2D nmap : register(t1);
SamplerState splr : register(s0);

#include "Lighting/BRDF.hlsli"

float4 main(v2f i) : SV_Target
{
    float2 screenPos = (i.screenPos.xy * float2(1, -1) / i.screenPos.w) * 0.5 + 0.5;
    //screenPos = i.pos.xy / i.pos.w;
    float4 specularLight = SpecularLightingRT.Sample(splr, screenPos);
    float4 diffuseLight = DiffuseLightingRT.Sample(splr, screenPos);
    
    float3 combinedLight = specularLight.rgb + diffuseLight.rgb;
    
    float4 diffuseTex = tex.Sample(splr, i.uv0);
    diffuseTex.rgb *= combinedLight;
    
    //return float4(0, 0, 1, 1);
    //return _Time.x;
    //return screenPos.y;
    //return combinedLight.rgbb;
    //return float4(materialColor.rgb, 1);
    return float4(diffuseLight.rgb * materialColor.rgb, 1);
    return diffuseTex;
}