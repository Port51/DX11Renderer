#include "./Common.hlsli"
#include "./PhongCommon.hlsli"
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

Texture2D tex : register(t0);
Texture2D nmap : register(t1);
SamplerState splr : register(s0);

float4 main(v2f i) : SV_Target
{
    i.normalVS = normalize(i.normalVS);
    i.tangentVS = normalize(i.tangentVS);
    float3 bitangentVS = cross(i.tangentVS, i.normalVS);
    float3x3 tbnMatrix = float3x3(i.tangentVS.xyz, bitangentVS.xyz, i.normalVS.xyz);

    float4 diffuseTex = tex.Sample(splr, i.uv0);

    // Calculate procedural normals
    {
        // todo: get normalmaps and use whiteout blending
        const float2 baseUV = (i.positionWS.xz + _Time.x * 4.0f * float2(10.29303f, 0.9821f)) * float2(0.01f, 0.1f);
        const float sampleOffset = 0.005f;
        const float n0 = tex.Sample(splr, baseUV).x;
        const float n1 = tex.Sample(splr, baseUV + float2(sampleOffset, 0.0)).x;
        const float n2 = tex.Sample(splr, baseUV + float2(0.0, sampleOffset)).x;

        const float scale = 300.0f;
        float3 n = float3((n1 - n0) / sampleOffset, (n2 - n1) / sampleOffset, scale);
        n = mul(n, tbnMatrix); // no need to transpose
        i.normalVS = normalize(n);
    }

    const float foamScale = 0.35;
    const float foamTex0 = tex.Sample(splr, i.positionWS.xz * foamScale + _Time.x * 1.13891).x;
    const float foamTex1 = tex.Sample(splr, i.positionWS.xz * foamScale * 0.8924 - _Time.x + float2(0.24839, 0.78214)).x;
    float foam = saturate((i.positionWS.y + 7.05) * 1.1);
    foam *= lerp(foamTex0 * foamTex1, 1.0, foam);
    foam = SCurve(foam);

    return float4(i.normalVS.xy * 0.5f + 0.5f, roughness, reflectivity * (1.0 - foam));
}