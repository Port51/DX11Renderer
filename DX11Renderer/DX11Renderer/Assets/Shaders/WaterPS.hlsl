#include "./Common.hlsli"
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
    float4 diffuseTex = 1.0;

    const float foamScale = 0.35;
    const float foamTex0 = tex.Sample(splr, i.positionWS.xz * foamScale + _Time.x * 1.13891).x;
    const float foamTex1 = tex.Sample(splr, i.positionWS.xz * foamScale * 0.8924 - _Time.x + float2(0.24839, 0.78214)).x;
    float foam = saturate((i.positionWS.y + 6.05) * 1.1);
    foam *= lerp(foamTex0 * foamTex1, 1.0, foam);
    foam = SCurve(foam);
    //return foam;

    //return float4(0, 0, 1, 1);
    //return _Time.x;
    //return screenPos.y;
    //return combinedLight.rgbb;
    //return float4(materialColor.rgb, 1);
    return float4(combinedLight.rgb * lerp(materialColor.rgb, 3.0, foam).rgb, 1) * diffuseTex;
    return diffuseTex;
}