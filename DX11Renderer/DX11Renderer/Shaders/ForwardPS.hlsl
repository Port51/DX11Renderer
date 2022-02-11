
//#define DEBUG_VIEW_CLUSTER_XY
//#define DEBUG_VIEW_CLUSTER_Z
//#define DEBUG_VIEW_LIGHT_COUNTS
//#define DEBUG_VIEW_LIGHT_COUNTS_AND_RANGES
//#define SHOW_LIGHT_LIMITS

#include "./CbufCommon.hlsli"
#include "./PhongCommon.hlsli"
#include "./Lighting/Lights.hlsli"
#include "./Lighting/HybridLightingCommon.hlsli"

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

cbuffer ClusterLight_CB : register(b4)
{
    uint4 _ClusterGroupResolutions; // groupsX, groupsY, groupsZ, padding
};

Texture2D tex : register(t2);
Texture2D nmap : register(t3);
StructuredBuffer<uint> ClusteredIndices : register(t4);
Texture2D<float> DitherTex : register(t5);
StructuredBuffer<StructuredLight> lights : register(t6);
StructuredBuffer<StructuredShadow> shadowData : register(t7);
Texture2D<float> ShadowAtlas : register(t8);
SamplerState splr : register(s0);
SamplerComparisonState ShadowAtlasSampler : register(s1);

#include "Lighting/BRDF.hlsli"

float4 main(v2f i) : SV_Target
{
    float3 positionNDC = i.positionNDC.xyz / i.positionNDC.w;
    float linearDepth = LinearEyeDepth(i.pos.z, _ZBufferParams);
    linearDepth = i.positionVS.z;
    
    float4 diffuseTex = tex.Sample(splr, i.uv0);
    
    uint3 cluster = GetClusterFromNDC(positionNDC, linearDepth);
    if (cluster.x >= _ClusterGroupResolutions.x || cluster.y >= _ClusterGroupResolutions.y)
    {
        return half4(0, 1, 0, 1);
    }
    uint clusterIdx = GetClusterIdx(_ClusterGroupResolutions.xyz, cluster);
    uint clusterDataIdx = GetClusterDataIdx(clusterIdx);
    
    float dither = DitherTex.Load(int3(GetPixelCoordFromNDC(positionNDC) % 8u, 0));
    
    // Light loop
    uint lightCt = ClusteredIndices[clusterDataIdx];
    float4 debugViews = 0.f;
    float3 diffuseLight = 0.f;
    for (uint li = 1u; li <= lightCt; ++li) // intentionally start from 1 to save an addition
    {
        StructuredLight light = lights[ClusteredIndices[clusterDataIdx + li]];
        float lightAtten = GetLightAttenuation(light, shadowData, ShadowAtlas, ShadowAtlasSampler, i.normalVS, i.positionVS, i.positionWS, dither, debugViews);
        
        lightAtten *= light.color_intensity.w;
        float3 lightColorInput = saturate(light.color_intensity.rgb * lightAtten);
        diffuseLight += lightColorInput;
    }
  
#if defined(DEBUG_VIEW_CLUSTER_XY)
    // Create RGB debug values based on cluster XY coords
    float3 debugClusterRGB = float3(frac(cluster.xy * 0.27841f), frac(dot(cluster.xy, float2(0.1783f, 0.5782f))));
    return float4(debugClusterRGB.rgb, 1.f);
#elif defined(DEBUG_VIEW_CLUSTER_Z)
    // Create RGB debug values based on cluster Z coord
    float debugClusterZ = (float)cluster.z / _ClusterGroupResolutions.z;
    float3 debugClusterRGB = frac(cluster.z * float3(0.12894f, 0.44832f, 0.8342f));
    return float4(debugClusterRGB.rgb, 1.f); // (lightCt > 0 ? 1.f : 0.5f)
#elif defined(DEBUG_VIEW_LIGHT_COUNTS)
    return float4((float3)lightCt / MAX_LIGHTS_PER_CLUSTER, 1.f);
#elif defined(DEBUG_VIEW_LIGHT_COUNTS_AND_RANGES)
    return float4(debugViews.z, (float)lightCt / MAX_LIGHTS_PER_CLUSTER, 0.f, 1.f);
#endif
    
    return diffuseLight.rgbb;
}