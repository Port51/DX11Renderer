#ifndef _SHADOWS_INCLUDED
#define _SHADOWS_INCLUDED

#include "./LightStructs.hlsli"

float GetSpotlightShadowAttenuation(StructuredShadow shadow, Texture2D<float> shadowMap, float3 positionVS)
{
    // Debug shadowmap output
    float4 shadowNDC = mul(shadow.shadowMatrix, float4(positionVS, 1));
    shadowNDC.xyz /= shadowNDC.w;
    float2 shadowUV = shadowNDC.xy * float2(0.5, -0.5) + 0.5;

    float isInShadow = 0;
    const float ShadowBias = 0.001;
    #if defined(USE_HARD_SHADOWS)
        float shadowDepth = shadowMap.SampleLevel(ShadowMapSampler, shadowUV.xy, 0);
        isInShadow = (shadowDepth + ShadowBias < shadowNDC.z);
    #elif defined(USE_HARDWARE_PCF)
        // Ref: https://takinginitiative.wordpress.com/2011/05/25/directx10-tutorial-10-shadow-mapping-part-2/
    
        float sum = 0;
 
        #if (PCF_TAPS > 0)
            [unroll]
            for (int y = -PCF_TAPS; y <= PCF_TAPS; y += 1)
            {
                [unroll]
                for (int x = -PCF_TAPS; x <= PCF_TAPS; x += 1)
                {
                    sum += ShadowMaps[0].SampleCmpLevelZero(ShadowMapSampler, shadowUV.xy, shadowNDC.z - ShadowBias, int2(x, y));
                }
            }
            isInShadow = sum / ((PCF_TAPS + 1) * (PCF_TAPS + 1));
        #else
            // 1-top PCF
            isInShadow = ShadowMaps[0].SampleCmpLevelZero(ShadowMapSampler, shadowUV.xy, shadowNDC.z - ShadowBias);
        #endif
    #endif
    
    isInShadow *= (shadowNDC.z > 0.0) * (shadowNDC.z < 1.0);
    return isInShadow;
}

#endif