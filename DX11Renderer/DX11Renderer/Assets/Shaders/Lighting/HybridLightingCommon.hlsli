#ifndef _HYBRID_LIGHTING_COMMON_INCLUDED
#define _HYBRID_LIGHTING_COMMON_INCLUDED

// Set cluster method here:
#define CLUSTER_USE_FRUSTUM_TILES // Frustums match screen pixels and use a combination of frustum and AABB intersection tests. This increases false positives from intersections, but allows culling with the Hi-Z buffer.
//#define CLUSTER_USE_AABB_TILES    // Clusters are rectangles defined by far plane of each cluster depth. These rectangles don't overlap, which lowers false positives. Clusters don't match pixels.

#define CLUSTER_DIMENSION           16u
#define INV_CLUSTER_DIMENSION       0.0625f
#define DEPTH_SLICES                16u
#define INV_DEPTH_SLICES            0.0625f
#define MAX_LIGHTS_PER_CLUSTER      32u
#define MAX_LIGHTS_PER_CLUSTER_PLUS 33u

#include "./../Common.hlsli"
#include "./LightingStructs.hlsli"
#include "./Shadows.hlsli"

SphereBounds GetSphereBoundsFromLight(StructuredLight light)
{
    SphereBounds result;
    
    [branch]
    if (light.data0.x == 0)
    {
        // Point light:
        result.positionVS = light.positionVS_range.xyz;
        result.radius = light.positionVS_range.w;
    }
    else if (light.data0.x == 1)
    {
        // Spotlight:
        // Project sphere halfway along cone
        result.positionVS = light.positionVS_range.xyz + light.direction.xyz * light.positionVS_range.w * 0.5f;
        result.radius = light.positionVS_range.w * 0.5f;
    }
    else
    {
        // Directional light:
        result.positionVS = 0.f;
        result.radius = 0.f;
    }
    return result;
}

// Given NDC coords and depth ranges, construct an AABB encapsulating the frustum
AABB GetFrustumAABBFromNDC(float4 planeNDC, float zNear, float zFar)
{
    AABB result;
    
    // For each side, select depth in order to get min/max of frustum points
    const float4 frustumPointSelection = float4(
        min(planeNDC.z * zNear, planeNDC.z * zFar),
        max(planeNDC.w * zNear, planeNDC.w * zFar),
        max(planeNDC.x * zNear, planeNDC.x * zFar),
        min(planeNDC.y * zNear, planeNDC.y * zFar)
    );
    
    const float3 minAABB = float3(_FrustumCornerDataVS.xy * frustumPointSelection.xy, zNear);
    const float3 maxAABB = float3(_FrustumCornerDataVS.xy * frustumPointSelection.zw, zFar);
    
    result.centerVS = (minAABB + maxAABB) * 0.5f;
    result.extentsVS = (maxAABB - minAABB) * 0.501f;
    return result;
}

// Given NDC coords and depth ranges, construct an AABB encapsulating the frustum
// This version avoids overlap, and should be used for clustered lighting but NOT tiled lighting
AABB GetConservativeFrustumAABBFromNDC(float4 planeNDC, float zNear, float zFar)
{
    AABB result;
    
    const float3 minAABB = float3(_FrustumCornerDataVS.xy * planeNDC.zw * zFar, zNear);
    const float3 maxAABB = float3(_FrustumCornerDataVS.xy * planeNDC.xy * zFar, zFar);
    
    result.centerVS = (minAABB + maxAABB) * 0.5f;
    result.extentsVS = (maxAABB - minAABB) * 0.501f;
    return result;
}

bool AABBSphereIntersection(float3 spherePos, float sphereRad, float3 aabbCenter, float3 aabbExtents)
{
    float3 displ = max(0, abs(aabbCenter - spherePos) - aabbExtents);
    float sdfSqr = dot(displ, displ);
    return sdfSqr <= sphereRad * sphereRad;
}

float GetClusterZNear(uint zSlice)
{
    // EQ: Z = NearZ (FarZ / NearZ) ^ (slice / numSlices)
    // From: http://www.aortiz.me/2018/12/21/CG.html#part-2 and DOOM 2016 SIGGRAPH
    
    // _ProjectionParams: Y = near, Z = far
    // _ZBufferParams: Y = far / near
    return _ProjectionParams.y * pow(_ZBufferParams.y, (float) zSlice * INV_DEPTH_SLICES);
}

uint GetClusterIdx(uint3 groupResolutions, uint3 cluster)
{
    return (groupResolutions.y * cluster.x + cluster.y) * groupResolutions.z + cluster.z;
}

uint GetClusterSlice(float linearDepth)
{
    // EQ: floor( log(Z) * numSlices / (log(FarZ/NearZ)) - numSlices * log(NearZ) / log(FarZ/NearZ) )
    // From: http://www.aortiz.me/2018/12/21/CG.html#part-2 and DOOM 2016 SIGGRAPH
    
    // _ProjectionParams: Y = near, Z = far
    // _ZBufferParams: Y = far / near
    return (uint)max(log2(linearDepth) * _ClusterPrecalc.x + _ClusterPrecalc.y, 0.f);
}

uint2 GetClusterXYFromNDC(float3 positionNDC)
{
    // Full EQ: floor((positionNDC.xy * 0.5f + 0.5f) * _ScreenParams.xy * INV_CLUSTER_DIMENSION);
    // Below is an optimized version where most calculations are done on the CPU
    return (uint2) (positionNDC.xy * _ClusterXYRemap.xy + _ClusterXYRemap.xy); // happens to multiply and add by same value
}

uint3 GetClusterFromNDC(float3 positionNDC, float linearDepth)
{
    uint3 cluster;
    cluster.z = GetClusterSlice(linearDepth);
    
#if defined(CLUSTER_USE_AABB_TILES)
    // Reproject NDC to match cluster system
    // This is required as clusters are tightly packed AABBs without overlap, rather than frustums
    // A pixel may be located outside the frustum of a cluster, but within the AABB
    float projectionRatio = linearDepth / GetClusterZNear(cluster.z + 1u); // todo: calculate ZNear and 1/ZNear for each slice on CPU
    float3 clusterNDC = positionNDC * projectionRatio;
    cluster.xy = GetClusterXYFromNDC(clusterNDC);
#elif defined(CLUSTER_USE_FRUSTUM_TILES)
    // No reprojection needed in this case
    positionNDC.y *= -1.f;
    cluster.xy = GetClusterXYFromNDC(positionNDC);
#endif
    return cluster;
}

uint GetClusterDataIdx(uint clusterIdx)
{
    return MAX_LIGHTS_PER_CLUSTER_PLUS * clusterIdx;
}

float GetLightAttenuation(StructuredLight light, StructuredBuffer<StructuredShadow> shadowData, Texture2D<float> shadowAtlas, SamplerComparisonState shadowAtlasSampler, float3 normalVS, float3 positionVS, float3 positionWS, float dither, inout float4 debugViews)
{
    debugViews = float4(0.f, 1.f, 0.f, 0.f); // cascades, all shadow atten, lights in range, -
    
    float lightAtten;
    float3 lightDirVS;
        
    uint type = (uint) light.data0.x;
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
            debugViews.x = GetShadowCascade(positionVS, dither) * 0.25f;
//#endif
                
            float shadowAtten = GetDirectionalShadowAttenuation(shadowData[shadowCascadeIdx], shadowAtlas, shadowAtlasSampler, positionVS, normalVS, NdotL);
            lightAtten *= shadowAtten;
            debugViews.y *= shadowAtten;
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
                float shadowAtten = GetSpotlightShadowAttenuation(shadowData[shadowIdx], shadowAtlas, shadowAtlasSampler, positionVS, normalVS, NdotL);
                shadowAtten = saturate(shadowAtten + outOfRange);
                lightAtten *= shadowAtten;
                debugViews.y *= shadowAtten;
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
            float shadowAtten = GetSpotlightShadowAttenuation(shadowData[shadowFaceIdx], shadowAtlas, shadowAtlasSampler, positionVS, normalVS, NdotL);
            shadowAtten = saturate(shadowAtten + outOfRange);
            lightAtten *= shadowAtten;
            debugViews.y *= shadowAtten;
        }
            
#if defined(DEBUG_VIEW_LIGHT_COUNTS_AND_RANGES)
        // This debug view shows a solid color if in light range
        debugViews.z += (lightDist < light.positionVS_range.w);
#endif
    }
    
    return lightAtten;
}


#endif