#ifndef _SHADOWS_INCLUDED
#define _SHADOWS_INCLUDED

// Ref: https://stackoverflow.com/questions/17123735/how-to-enable-hardware-percentage-closer-filtering
SamplerComparisonState ShadowAtlasSampler : register(s0);
Texture2D<float> ShadowAtlas : register(t4);



#include "./LightStructs.hlsli"

float2 GetShadowTileUV(float4 shadowNDC, uint2 tile)
{
    // Select tile
    shadowNDC.xy = shadowNDC.xy * float2(0.5, -0.5) + tile - 0.5;
    // Remap to 0-1 range
    return shadowNDC.xy * float2(0.5, 0.5) + 0.5;
}

float GetSpotlightShadowAttenuation(StructuredShadow shadow, float3 positionVS, float3 normalVS, float NdotL)
{
    float3 normalBiasVS = normalVS * (NdotL * -0.065 + 0.075); // Large bias at grazing angles, small bias when light dir is similar to normal
    float4 shadowNDC = mul(shadow.shadowMatrix, float4(positionVS + normalBiasVS, 1));
    shadowNDC.xyz /= shadowNDC.w;
    
    float2 shadowUV = GetShadowTileUV(shadowNDC, shadow.tile);
    //return shadowUV.x * (shadowNDC.z > 0.0) * (shadowNDC.z < 1.0);

    float isInShadow = 0;
    const float ShadowBias = 0.001;
    //#if defined(USE_HARD_SHADOWS)
    isInShadow = ShadowAtlas.SampleCmpLevelZero(ShadowAtlasSampler, shadowUV.xy, shadowNDC.z - ShadowBias);
    //isInShadow = isInShadow > shadowNDC.z;
        //float shadowDepth = shadowMap.SampleLevel(ShadowMapSampler, shadowUV.xy, 0);
        //float shadowDepth = tex2Dlod(ShadowMapSampler, float4(shadowUV.xy, 0, 0));
    /*#elif defined(USE_HARDWARE_PCF)
        // Ref: https://takinginitiative.wordpress.com/2011/05/25/directx10-tutorial-10-shadow-mapping-part-2/
    
        float sum = 0;
 
        #if (PCF_TAPS > 0)
            [unroll]
            for (int y = -PCF_TAPS; y <= PCF_TAPS; y += 1)
            {
                [unroll]
                for (int x = -PCF_TAPS; x <= PCF_TAPS; x += 1)
                {
                    sum += ShadowAtlas.SampleCmpLevelZero(ShadowMapSampler, shadowUV.xy, shadowNDC.z - ShadowBias, int2(x, y));
                }
            }
            isInShadow = sum / ((PCF_TAPS + 1) * (PCF_TAPS + 1));
        #else
            // 1-top PCF
            isInShadow = ShadowAtlas.SampleCmpLevelZero(ShadowMapSampler, shadowUV.xy, shadowNDC.z - ShadowBias);
        #endif
    #endif*/
    
    isInShadow *= (shadowNDC.z > 0.0) * (shadowNDC.z < 1.0);
    return isInShadow;
}

#endif