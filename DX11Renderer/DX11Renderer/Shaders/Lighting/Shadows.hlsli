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

float GetSpotlightShadowAttenuation(StructuredShadow shadow, float3 positionVS, float3 normalVS, float NdotL)
{
    // Apply large bias at grazing angles, small bias when light dir is similar to normal
    float3 normalBiasVS = normalVS * (NdotL * -0.075 + 0.085);
    float4 shadowNDC = mul(shadow.shadowMatrix, float4(positionVS + normalBiasVS, 1));
    shadowNDC.xyz /= shadowNDC.w;
    
    float2 shadowUV = GetShadowTileUV(shadowNDC, shadow.tile);
    
    float isInShadow = 0;
    const float ShadowBias = 0.001;
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
    isInShadow = sum / SHADOW_PCF_TOTAL_TAPS;
#else
    // Fallback to hard shadows
    isInShadow = ShadowAtlas.SampleCmpLevelZero(ShadowAtlasSampler, shadowUV.xy, shadowNDC.z - ShadowBias);
#endif
    
    // Limit to current tile
    isInShadow *= (all(shadowNDC.xy > -1.0) * all(shadowNDC.xyz < 1.0) * (shadowNDC.z > 0.0));
    return isInShadow;
}

#endif