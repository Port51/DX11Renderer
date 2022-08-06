
#include "./CbufCommon.hlsli"
#include "./PhongCommon.hlsli"

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

    // Fresnel
    float3 viewDirVS = normalize(i.positionVS);
    float fresnel = pow(saturate(1.0 - dot(i.normalVS, -viewDirVS)), 1.5f);

    // Color selection
    const float3 skinColor = float3(255.f / 256.f, 174.f / 256.f, 0.f / 256.f);
    const float3 clothColor = float3(0.f / 256.f, 51.f / 256.f, 256.f / 256.f);
    const float3 staffColor = float3(122.f / 256.f, 37.f / 256.f, 0.f / 256.f);
    const float3 magicColor = float3(0, 0, 0);
    float3 color = lerp(magicColor, lerp(skinColor, lerp(staffColor, clothColor, i.vertColor.g), i.vertColor.r), i.vertColor.b);

    // Glowy gem on end of staff
    float3 emission = float3(32.f / 256.f, 255.f / 256.f, 47.f / 256.f) * (1.0 - i.vertColor.b) * (0.2f + 2.8f * fresnel);

    return float4(combinedLight.rgb * color + emission, 1);
}