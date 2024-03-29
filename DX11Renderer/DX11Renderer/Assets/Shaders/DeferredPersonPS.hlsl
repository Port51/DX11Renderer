
#include "./CbufCommon.hlsli"
#include "./PhongCommon.hlsli"

cbuffer LightCBuf : register(b0)
{
    float3 lightPos;
    float lightInvRangeSqr;
    float3 lightColor;
    float lightIntensity;
};

// Per-frame
Texture2D SpecularLightingRT : register(t5);
Texture2D DiffuseLightingRT : register(t6);

// Per-draw
Texture2D tex : register(t0);
Texture2D nmap : register(t1);
SamplerState splr : register(s0);

#include "./Lighting/BRDF.hlsli"

float4 main(v2f i) : SV_Target
{
    float3 positionNDC = i.positionNDC.xyz / i.positionNDC.w;
    float2 screenPos = positionNDC.xy * float2(0.5f, -0.5f) + 0.5;
    float4 specularLight = SpecularLightingRT.Sample(splr, screenPos);
    float4 diffuseLight = DiffuseLightingRT.Sample(splr, screenPos);

    float3 combinedLight = specularLight.rgb + diffuseLight.rgb;

    // Fresnel
    float3 viewDirVS = normalize(i.positionVS);
    float fresnel = pow(saturate(1.0 - dot(i.normalVS, -viewDirVS)), 1.5f);

    // Color selection
    const float3 interiorColor = 0.f;// float3(255.f / 255.f, 174.f / 255.f, 0.f / 255.f);
    const float3 clothColor = float3(255.f / 255.f, 37.f / 255.f, 0.f / 255.f); // 0.135f;
    const float3 staffColor = float3(122.f / 255.f, 37.f / 255.f, 0.f / 255.f);
    const float3 magicColor = float3(0, 0, 0);
    float3 color = lerp(magicColor, lerp(interiorColor, lerp(staffColor, clothColor, i.vertColor.g), i.vertColor.r), i.vertColor.b);

    // Glowy gem on end of staff
    float3 emission = float3(0.f / 255.f, 98.f / 255.f, 255.f / 255.f) * ((1.0f - i.vertColor.b) * (0.2f + 2.8f * fresnel) * 2.0f);

    return float4(combinedLight.rgb * color + emission, 1);
}