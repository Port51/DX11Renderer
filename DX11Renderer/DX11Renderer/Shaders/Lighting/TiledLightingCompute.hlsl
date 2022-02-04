#include "./../CbufCommon.hlsli"
#include "./../Lighting/LightStructs.hlsli"
#include "./../Lighting/Lights.hlsli"
#include "./../Lighting/BRDF.hlsli"

// References:
// https://wickedengine.net/2018/01/10/optimizing-tile-based-light-culling/

// Shadow settings:
//#define SHADOW_PCF_3_TAP
//#define SHADOW_PCF_5_TAP
#define SHADOW_PCF_7_TAP
//#define USE_HARD_SHADOWS

#define MAX_TILE_LIGHTS     64
#define TILED_GROUP_SIZE    16
#define MAX_SHADOWS         4

#define USE_HI_Z_BUFFER

#define USE_FRUSTUM_INTERSECTION_TEST
#define USE_AABB_INTERSECTION_TEST

//
// Debug views
//

#define DEBUG_VIEW_ALL_SHADOWS
//#define DEBUG_VIEW_SHADOW
//#define DEBUG_VIEW_LIGHT_COUNTS
#define DEBUG_VIEW_LIGHT_COUNTS_AND_RANGES
//#define DEBUG_VIEW_VERIFY_AABB_BOUNDS
//#define DEBUG_VIEW_CASCADE_IDX
//#define DEBUG_VIEW_GEOMETRY

// Inputs
StructuredBuffer<StructuredLight> lights : register(t0);
StructuredBuffer<StructuredShadow> shadowData : register(t1);
Texture2D<float4> NormalRoughReflectivityRT : register(t2);
Texture2D<float> DepthRT : register(t3);
Texture2D<float2> HiZBuffer : register(t4);
Texture2D<float> ShadowAtlas : register(t5);
Texture2D<float> DitherTex : register(t6);

SamplerComparisonState ShadowAtlasSampler : register(s0);

#include "./../Lighting/Shadows.hlsli"

// Outputs
RWTexture2D<float4> SpecularLightingOut : register(u0);
RWTexture2D<float4> DiffuseLightingOut : register(u1);
RWTexture2D<float4> DebugOut : register(u2);

groupshared uint tileLightCount;
groupshared uint tileLightIndices[MAX_TILE_LIGHTS];
groupshared uint minTileZ;
groupshared uint maxTileZ;

/*float ConvertZToLinearDepth(float depth)
{
    float near = camera_near;
    float far = camera_far;
    
    return (near * far) / (far - depth * (far - near));

}*/

// Z buffer to linear depth.
// Does NOT correctly handle oblique view frustums.
// Does NOT work with orthographic projection.
// zBufferParam = { (f-n)/n, 1, (f-n)/n*f, 1/f }
float LinearEyeDepth(float depth, float4 zBufferParam)
{
    return 1.0 / (zBufferParam.z * depth + zBufferParam.w);
}

float RawDepthToLinearDepth(float rawDepth)
{
    // See: https://forum.unity.com/threads/getting-scene-depth-z-buffer-of-the-orthographic-camera.601825/#post-4966334
    float persp = LinearEyeDepth(rawDepth, _ZBufferParams);
    float ortho = (_ZBufferParams.z - _ZBufferParams.y) * (1 - rawDepth) + _ZBufferParams.y;
    return lerp(persp, ortho, _OrthoParams.w);
}

bool AABBSphereIntersection(float3 spherePos, float sphereRad, float3 aabbCenter, float3 aabbExtents)
{
    float3 displ = max(0, abs(aabbCenter - spherePos) - aabbExtents);
    float sdfSqr = dot(displ, displ);
    return sdfSqr <= sphereRad * sphereRad;
}

[numthreads(TILED_GROUP_SIZE, TILED_GROUP_SIZE, 1)]
void CSMain(uint3 gId : SV_GroupID, uint gIndex : SV_GroupIndex, uint3 groupThreadId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
    //
    // Cull lights
    //
    
    if (gIndex == 0)
    {
        tileLightCount = 0u;
        minTileZ = asuint(10000.f);
        maxTileZ = asuint(0.f);
    }
    
    float dither = DitherTex.Load(int3(tId.xy % 8, 0));
    
    GroupMemoryBarrierWithGroupSync();
    
#if defined(USE_HI_Z_BUFFER)
    const float linearDepth = HiZBuffer.Load(int3(tId.xy, 0)).r * _ProjectionParams.z;
    const float2 tileDepthRange = HiZBuffer.Load(int3(tId.xy / 16, 4)).rg * _ProjectionParams.z;
    const float isGeometry = linearDepth < 10000.f;
#else
    const float rawDepth = DepthRT.Load(int3(tId.xy, 0));
    const float isGeometry = rawDepth < 1.f;
    const float linearDepth = RawDepthToLinearDepth(rawDepth);
    const uint intDepth = asuint(linearDepth);
    InterlockedMin(minTileZ, intDepth);
    InterlockedMax(maxTileZ, intDepth);
    
    GroupMemoryBarrierWithGroupSync();
    
    const float2 tileDepthRange = float2(asfloat(minTileZ), asfloat(maxTileZ));
#endif
    
    // todo: early out if only sky
    
    float debugValue = 0.f;
    
    // Calculate NDC for tile corners
    const float4 planeNDC = ((float4) (gId.xyxy * TILED_GROUP_SIZE + float4(TILED_GROUP_SIZE, 0.f, 0.f, TILED_GROUP_SIZE)) * _ScreenParams.zwzw) * 2.f - 1.f;

#if defined(USE_FRUSTUM_INTERSECTION_TEST)
    // Derive frustum planes
    // #0 faces to the right, and the rest continue in CCW fashion
    float3 frustumPlanes[4];
    
    // Use cross product to turn tile view directions into plane directions
    // The cross product is done by flipping X or Y with Z
    frustumPlanes[0] = normalize(float3(1.f, 0.f, -planeNDC.x * _FrustumCornerDataVS.x));
    frustumPlanes[1] = normalize(float3(0.f, 1.f, -planeNDC.y * _FrustumCornerDataVS.y));
    frustumPlanes[2] = normalize(float3(-1.f, 0.f, planeNDC.z * _FrustumCornerDataVS.x));
    frustumPlanes[3] = normalize(float3(0.f, -1.f, planeNDC.w * _FrustumCornerDataVS.y));
#endif
    
#if defined(USE_AABB_INTERSECTION_TEST)
    // For each side, select depth in order to get min/max of frustum points
    const float4 frustumPointSelection = float4(
        min(planeNDC.z * tileDepthRange.y, planeNDC.z * tileDepthRange.x),
        max(planeNDC.w * tileDepthRange.y, planeNDC.w * tileDepthRange.x),
        max(planeNDC.x * tileDepthRange.y, planeNDC.x * tileDepthRange.x),
        min(planeNDC.y * tileDepthRange.y, planeNDC.y * tileDepthRange.x)
    );
    const float3 minAABB = float3(_FrustumCornerDataVS.xy * frustumPointSelection.xy, tileDepthRange.x);
    const float3 maxAABB = float3(_FrustumCornerDataVS.xy * frustumPointSelection.zw, tileDepthRange.y);
    const float3 aabbCenter = (minAABB + maxAABB) * 0.5f;
    const float3 aabbExtents = (maxAABB - minAABB) * 0.501f;
#endif
    
    uint i;
    float debugAllShadowAtten = 1.f;
    float debugAllLightsInRange = 0.f;
    float debugCascade = 0.f;
    for (i = gIndex; i < _VisibleLightCount; i += TILED_GROUP_SIZE * TILED_GROUP_SIZE)
    {
        StructuredLight light = lights[i];
        
        bool inFrustum = true;
        
        // Calculate sphere to test against
        float4 sphereData;
        [branch]
        if (light.data0.x == 0)
        {
            // Point light:
            sphereData = light.positionVS_range;
        }
        else if (light.data0.x == 1)
        {
            // Spotlight:
            // Project sphere halfway along cone
            sphereData = float4(light.positionVS_range.xyz + light.direction.xyz * light.positionVS_range.w * 0.5, light.positionVS_range.w * 0.5);
        }
        else
        {
            // Directional light:
            sphereData = (float4)0.f;
        }
        
#if defined(USE_FRUSTUM_INTERSECTION_TEST)     
        [unroll]
        for (uint j = 0; j < 4; ++j)
        {
            float d = dot(frustumPlanes[j], sphereData.xyz);
            inFrustum = inFrustum && (d < sphereData.w);
        }
    #if !defined(USE_AABB_INTERSECTION_TEST)
        // Only need to test near and far planes if AABB test is not used
        inFrustum = inFrustum && (light.positionVS_range.z >= tileDepthRange.x - light.positionVS_range.w);
        inFrustum = inFrustum && (light.positionVS_range.z <= tileDepthRange.y + light.positionVS_range.w);
    #endif
        inFrustum = inFrustum || (light.data0.x == 2); // always show directional lights
#endif

#if defined(USE_AABB_INTERSECTION_TEST)
        // Sphere-AABB test
        inFrustum = inFrustum && AABBSphereIntersection(sphereData.xyz, sphereData.w, aabbCenter, aabbExtents);
#endif
        
        [branch]
        if (inFrustum)
        {
            uint idx;
            InterlockedAdd(tileLightCount, 1u, idx);
            tileLightIndices[idx] = i;
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    //
    // Apply lighting
    //
    float2 screenUV = tId.xy * _ScreenParams.zw;
    float2 positionNDC = screenUV * 2.f - 1.f;
    
    float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1.f);
    float3 positionVS = frustumPlaneVS * linearDepth;
    float3 viewDirVS = normalize(positionVS);
    
    float3 positionWS = mul(_InvViewMatrix, float4(positionVS.xyz, 1.f)).xyz;
    
    float4 gbufferTex = NormalRoughReflectivityRT[tId.xy];
    float3 normalVS;
    normalVS.xy = gbufferTex.xy * 2.f - 1.f;
    normalVS.z = -sqrt(1.f - dot(normalVS.xy, normalVS.xy)); // positive Z points away from the camera
    
    float linearRoughness = gbufferTex.z;
    float roughness = linearRoughness * linearRoughness;
    float reflectivity = gbufferTex.w;
    
    // f0 = fresnel reflectance at normal incidence
    // f90 = fresnel reflectance at grazing angles (usually 1)
    //float metalness = 1;
    //float3 reflectance = lerp(0.04, 1.0, metalness);
    float3 f0 = 1.f; //0.16 * reflectance * reflectance;
    
    // note: Filament uses f0 of 0.0-0.02 to indicate pre-baked specular, and turn off f90 in that case
    // For now, just set it to 1.0
    float f90 = 1.f;
    
    // https://gamedev.net/forums/topic/579417-forced-to-unroll-loop-but-unrolling-failed-resolved/4690649/
    float3 diffuseLight = 0.f;
    float3 specularLight = 0.f;
    for (i = 0; i < tileLightCount; ++i)
    {
        StructuredLight light = lights[tileLightIndices[i]];
        
        float lightAtten;
        float3 lightDirVS;
        
        uint type = (uint)light.data0.x;
        int shadowIdx = light.direction.w;
        [branch] // should be same for each thread group
        if (type == 2u)
        {
            // Directional light
            lightDirVS = light.direction.xyz;
            float NdotL = dot(normalVS, -lightDirVS.xyz);
            lightAtten = saturate(NdotL);
            
            [branch]
            if (shadowIdx != -1)
            {
                // Directional shadow
                
                // Choose cascade
                uint shadowCascadeIdx = shadowIdx + GetShadowCascade(positionVS, dither);
//#if defined(DEBUG_VIEW_CASCADE_IDX)
                debugCascade = GetShadowCascade(positionVS, dither) * 0.25f;
//#endif
                
                float shadowAtten = GetDirectionalShadowAttenuation(shadowData[shadowCascadeIdx], positionVS, normalVS, NdotL);
                lightAtten *= shadowAtten;
                debugAllShadowAtten *= shadowAtten;
            }
        }
        else
        {
            float3 displ = light.positionVS_range.xyz - positionVS.xyz;
            float lightDist = length(displ);
            lightDirVS = displ / max(lightDist, 0.0001f);
            float NdotL = dot(normalVS, -lightDirVS.xyz);
            float outOfRange = lightDist > light.positionVS_range.w;
            
            lightAtten = GetSphericalLightAttenuation(lightDist, light.data0.y, light.positionVS_range.w);
        
            [branch] // should be same for each thread group, as thread groups are the size of 1 tile
            if (type == 1u)
            {
                // Apply spotlight cone
                float3 spotlightDir = normalize(light.direction.xyz);
            
                // todo: replace with [MAD] op, depending on light culling?
                float spotCos = dot(spotlightDir, -lightDirVS);
                lightAtten *= saturate((spotCos - light.data0.w) / (light.data0.z - light.data0.w));
                [branch]
                if (shadowIdx != -1)
                {
                    // Spotlight shadow
                    float shadowAtten = GetSpotlightShadowAttenuation(shadowData[shadowIdx], positionVS, normalVS, NdotL);
                    shadowAtten = saturate(shadowAtten + outOfRange);
                    lightAtten *= shadowAtten;
                    debugAllShadowAtten *= shadowAtten;
                }
            }
            else if (shadowIdx != -1)
            {
                // Point light shadow
                
                // Get cubemap face based on WS offset
                float3 lightPosWS = mul(_InvViewMatrix, float4(light.positionVS_range.xyz, 1.f)).xyz;
                float3 offsetWS = positionWS - lightPosWS;
                uint shadowFaceIdx = shadowIdx + GetCubeMapFaceID(offsetWS);
                
                // Attenuation
                float shadowAtten = GetSpotlightShadowAttenuation(shadowData[shadowFaceIdx], positionVS, normalVS, NdotL);
                shadowAtten = saturate(shadowAtten + outOfRange);
                lightAtten *= shadowAtten;
                debugAllShadowAtten *= shadowAtten;
            }
            
#if defined(DEBUG_VIEW_LIGHT_COUNTS_AND_RANGES)
            // This debug view shows a solid color if in light range
            debugAllLightsInRange += (lightDist < light.positionVS_range.w);
#endif
        }
        
        lightAtten *= light.color_intensity.w;
        float3 lightColorInput = saturate(light.color_intensity.rgb * lightAtten);
        
        BRDFLighting brdf = BRDF(f0, f90, roughness, linearRoughness, normalVS, viewDirVS, lightDirVS);
        diffuseLight += lightColorInput;
        //diffuseLight += brdf.diffuseLight * lightColorInput;
        specularLight += brdf.specularLight * lightColorInput;
    }
    
    diffuseLight *= isGeometry;
    specularLight *= isGeometry;
    
    float3 debugColor = debugValue;
#if defined(DEBUG_VIEW_LIGHT_COUNTS)
    debugColor = float3((diffuseLight.r) * isGeometry, tileLightCount * 0.2, 0);
    #define DEBUG_VIEW_SHOW_GRID
#elif defined(DEBUG_VIEW_VERIFY_AABB_BOUNDS)
    // Show green where position is within AABB, and red where it isn't
    // Entire screen should be green
    bool inBounds = all(positionVS.xyz >= minAABB.xyz) * all(positionVS.xyz <= maxAABB.xyz);
    debugColor = inBounds ? float3(0, 1, 0) : float3(1, 0, 0);
    #define DEBUG_VIEW_SHOW_GRID
#elif defined(DEBUG_VIEW_LIGHT_COUNTS_AND_RANGES)
    debugColor = float3(debugAllLightsInRange * isGeometry * 0.2, tileLightCount * 0.2, 0);
    #define DEBUG_VIEW_SHOW_GRID
#elif defined(DEBUG_VIEW_ALL_SHADOWS)
    debugColor = debugAllShadowAtten; // - debugCascade;
#elif defined(DEBUG_VIEW_SHADOW)
    // Show shadow for a light
    //debugColor = GetSpotlightShadowAttenuation(shadowData[6], positionVS, normalRough.xyz, dot(normalRough.xyz, -lights[1].direction.xyz));
    const uint ptLightMapIdx = 0u;
    debugColor = GetSpotlightShadowAttenuation(shadowData[ptLightMapIdx], positionVS, normalRough.xyz, dot(normalRough.xyz, PointLightFaceDirWS[ptLightMapIdx]));
#elif defined(DEBUG_VIEW_CASCADE_IDX)
    debugColor = debugCascade;
#elif defined(DEBUG_VIEW_GEOMETRY)
    debugColor = frac(positionVS.z * 3.0);
#endif
    debugColor = -normalVS.z;
    
#if defined(DEBUG_VIEW_SHOW_GRID)
    if (groupThreadId.x == 0 || groupThreadId.y == 0)
    {
        debugColor = float3(0.3, 0.1, 0.8);
    }
#endif
    
    SpecularLightingOut[tId.xy] = float4(specularLight, 1);
    DiffuseLightingOut[tId.xy] = float4(diffuseLight, 1);
    DebugOut[tId.xy] = float4(debugColor, 1);
}