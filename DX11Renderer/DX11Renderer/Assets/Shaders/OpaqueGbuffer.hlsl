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
    
    float3 n;
    if (normalMapEnabled && false)
    {
        const float3 normalSample = nmap.Sample(splr, i.uv0).xyz;
        n.x = normalSample.x * 2.0f - 1.0f;
        n.y = -normalSample.y * 2.0f + 1.0f; // Convert from OpenGL to DX style
        n.z = -normalSample.z;
        n = mul(n, tbnMatrix); // no need to transpose
        i.normalVS = normalize(-n); // fix sampling interpolation
    }
    
    return float4(i.normalVS.xy * 0.5f + 0.5f, roughness, reflectivity);
}