#ifndef _SHADOWS_INCLUDED
#define _SHADOWS_INCLUDED

#define SHADOW_ATLAS_TILES_PER_SIDE     4
#define SHADOW_ATLAS_TILE_SCALE         (1.f / SHADOW_ATLAS_TILES_PER_SIDE)

// Ref: https://stackoverflow.com/questions/17123735/how-to-enable-hardware-percentage-closer-filtering

#if defined(SHADOW_PCF_3_TAP)
    #define SHADOW_USE_PCF_FILTERING
    #define SHADOW_PCF_DIR_TAPS     1 // taps in each direction
    #define SHADOW_PCF_TOTAL_TAPS   9
#elif defined(SHADOW_PCF_5_TAP)
    #define SHADOW_USE_PCF_FILTERING
    #define SHADOW_PCF_DIR_TAPS     2 // taps in each direction
    #define SHADOW_PCF_TOTAL_TAPS   25
#elif defined(SHADOW_PCF_7_TAP)
    #define SHADOW_USE_PCF_FILTERING
    #define SHADOW_PCF_DIR_TAPS     3 // taps in each direction
    #define SHADOW_PCF_TOTAL_TAPS   49
#endif

//#define SHADOW_DEBUG_SHOW_DEPTH_MAP

static float3 PointLightFaceDirWS[] =
{
    float3(0, 0, 1),
    float3(1, 0, 0),
    float3(0, 0, -1),
    float3(-1, 0, 0),
    float3(0, 1, 0),
    float3(0, -1, 0),
};

SamplerComparisonState ShadowAtlasSampler : register(s0);
Texture2D<float> ShadowAtlas : register(t4);

#include "./LightStructs.hlsli"

float2 GetShadowTileUV(float4 shadowNDC, uint2 tile)
{
    // Select tile
    
    // Original version:
    //shadowNDC.xy = shadowNDC.xy * float2(SHADOW_ATLAS_TILE_SCALE, -SHADOW_ATLAS_TILE_SCALE) + tile * 2 * SHADOW_ATLAS_TILE_SCALE - (1 - SHADOW_ATLAS_TILE_SCALE);
    // Optimized version:
    shadowNDC.xy = (shadowNDC.xy * float2(1.f, -1.f) + tile * 2.f + 1.f) * SHADOW_ATLAS_TILE_SCALE - 1.f;
    
    // Remap to 0-1 range
    return shadowNDC.xy * float2(0.5, 0.5) + 0.5;
}

#define CUBEMAPFACE_POSITIVE_Z 0u
#define CUBEMAPFACE_POSITIVE_X 1u
#define CUBEMAPFACE_NEGATIVE_Z 2u
#define CUBEMAPFACE_NEGATIVE_X 3u
#define CUBEMAPFACE_POSITIVE_Y 4u
#define CUBEMAPFACE_NEGATIVE_Y 5u
uint GetCubeMapFaceID(float3 dir)
{
    if (abs(dir.z) >= abs(dir.x) && abs(dir.z) >= abs(dir.y))
    {
        return (dir.z < 0.0) ? CUBEMAPFACE_NEGATIVE_Z : CUBEMAPFACE_POSITIVE_Z;
    }
    else if (abs(dir.y) >= abs(dir.x))
    {
        return (dir.y < 0.0) ? CUBEMAPFACE_NEGATIVE_Y : CUBEMAPFACE_POSITIVE_Y;
    }
    else
    {
        return (dir.x < 0.0) ? CUBEMAPFACE_NEGATIVE_X : CUBEMAPFACE_POSITIVE_X;
    }
}

float GetSpotlightShadowAttenuation(StructuredShadow shadow, float3 positionVS, float3 normalVS, float NdotL)
{
    // Apply large bias at grazing angles, small bias when light dir is similar to normal
    float3 normalBiasVS = normalVS * (NdotL * -0.075 + 0.085) * 0;
    float4 shadowNDC = mul(shadow.shadowMatrix, float4(positionVS + normalBiasVS, 1));
    shadowNDC.xyz /= shadowNDC.w;
    
    float2 shadowUV = GetShadowTileUV(shadowNDC, shadow.tile);
    
    float shadowAtten = 0;
    const float ShadowBias = 0.001 * 0;
#if defined(SHADOW_USE_PCF_FILTERING)
    // PCF filtering
    float sum = 0;
    [unroll]
    for (int y = -SHADOW_PCF_DIR_TAPS; y <= SHADOW_PCF_DIR_TAPS; y += 1)
    {
        [unroll]
        for (int x = -SHADOW_PCF_DIR_TAPS; x <= SHADOW_PCF_DIR_TAPS; x += 1)
        {
            sum += ShadowAtlas.SampleCmpLevelZero(ShadowAtlasSampler, shadowUV.xy, shadowNDC.z - ShadowBias, int2(x, y));
        }
    }
    shadowAtten = sum / SHADOW_PCF_TOTAL_TAPS;
#else
    // Fallback to hard shadows
    shadowAtten = ShadowAtlas.SampleCmpLevelZero(ShadowAtlasSampler, shadowUV.xy, shadowNDC.z - ShadowBias);
#endif
    
#if defined(SHADOW_DEBUG_SHOW_DEPTH_MAP)
    shadowAtten = ShadowAtlas.Load(int3(shadowUV.xy * _ShadowAtlasTexelResolution.xy, 0u));
#endif
    
    // Limit to current tile
    shadowAtten = saturate(shadowAtten + any(shadowNDC.xy < -1.0) + any(shadowNDC.xyz > 1.0) + (shadowNDC.z < 0.0));
    return shadowAtten;
}

float CascadeDistSqr(float3 dist)
{
    return dot(dist, dist);
}

uint GetShadowCascade(float3 positionVS)
{
    // 0 is crazy
    // 1 looks better farther away (looking FROM light)
    // 2 works
    // 3 works when facing towards light
    
    //return 3u;
    
    [branch]
    if (CascadeDistSqr(positionVS - _ShadowCascadeSphere0.xyz) < _ShadowCascadeSphere0.w)
    {
        return 0u;
    }
    else if (CascadeDistSqr(positionVS - _ShadowCascadeSphere1.xyz) < _ShadowCascadeSphere1.w)
    {
        return 1u;
    }
    else if (CascadeDistSqr(positionVS - _ShadowCascadeSphere2.xyz) < _ShadowCascadeSphere2.w)
    {
        return 2u;
    }
    else
    {
        return 3u;
    }
}

float GetDirectionalShadowAttenuation(StructuredShadow shadow, float3 positionVS, float3 normalVS, float NdotL)
{
    // Apply large bias at grazing angles, small bias when light dir is similar to normal
    float3 normalBiasVS = normalVS * (NdotL * -0.075 + 0.085);
    float4 shadowNDC = mul(shadow.shadowMatrix, float4(positionVS + normalBiasVS, 1.f));
    shadowNDC.xyz /= shadowNDC.w;
    
    float2 shadowUV = GetShadowTileUV(shadowNDC, shadow.tile);
    //return shadow.tile.x * 0.25;
    //return shadow.shadowMatrix[0][0];
    //return shadowUV.x;
    //return (shadowNDC.x < 1.f) * (shadowNDC.x > -1.f);
    //return (shadowNDC.x * 0.5 + 0.5);
    
    float shadowAtten = 0;
    const float ShadowBias = 0.005f;
#if defined(SHADOW_USE_PCF_FILTERING)
    // PCF filtering
    float sum = 0;
    [unroll]
    for (int y = -SHADOW_PCF_DIR_TAPS; y <= SHADOW_PCF_DIR_TAPS; y += 1)
    {
        [unroll]
        for (int x = -SHADOW_PCF_DIR_TAPS; x <= SHADOW_PCF_DIR_TAPS; x += 1)
        {
            sum += ShadowAtlas.SampleCmpLevelZero(ShadowAtlasSampler, shadowUV.xy, shadowNDC.z - ShadowBias, int2(x, y));
        }
    }
    shadowAtten = sum / SHADOW_PCF_TOTAL_TAPS;
#else
    // Fallback to hard shadows
    shadowAtten = ShadowAtlas.SampleCmpLevelZero(ShadowAtlasSampler, shadowUV.xy, shadowNDC.z - ShadowBias);
#endif
    
#if defined(SHADOW_DEBUG_SHOW_DEPTH_MAP)
    shadowAtten = ShadowAtlas.Load(int3(shadowUV.xy * _ShadowAtlasTexelResolution.xy, 0u));
#endif
    
    // Limit to current tile
    shadowAtten = saturate(shadowAtten + any(shadowNDC.xy < -1.0) + any(shadowNDC.xyz > 1.0) + (shadowNDC.z < 0.0));
    return shadowAtten;
}

#endif