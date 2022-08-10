
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

#include "Lighting/BRDF.hlsli"

float4 main(v2f i) : SV_Target
{
    float3 positionNDC = i.positionNDC.xyz / i.positionNDC.w;
    float2 screenPos = positionNDC.xy * float2(0.5f, -0.5f) + 0.5;
    float4 specularLight = SpecularLightingRT.Sample(splr, screenPos);
    float4 diffuseLight = DiffuseLightingRT.Sample(splr, screenPos);

    float3 combinedLight = specularLight.rgb + diffuseLight.rgb;

    // Triplanar sampling
    const float diffuseScale = 0.15;
    float4 diffuseXZ = tex.Sample(splr, i.positionWS.xz * diffuseScale) * abs(i.normalWS.y);
    float4 diffuseXY = tex.Sample(splr, i.positionWS.xy * diffuseScale) * abs(i.normalWS.z);
    float4 diffuseYZ = tex.Sample(splr, i.positionWS.yz * diffuseScale) * abs(i.normalWS.x);

    float4 diffuseTex = lerp(1.0, diffuseXZ + diffuseXY + diffuseYZ, noiseIntensity);
    float isTop = saturate(i.normalWS.y * 1.75 - 0.35);

    return float4(combinedLight.rgb * lerp(float3(0.32, 0.11, 0.05), materialColor.rgb, isTop), 1) * diffuseTex;
}