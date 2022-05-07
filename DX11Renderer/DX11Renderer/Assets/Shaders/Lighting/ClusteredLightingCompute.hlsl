// References:
//  - http://www.aortiz.me/2018/12/21/CG.html#part-2

#define ENABLE_DEBUG_VIEWS // much faster if turned off
//#define DEBUG_VIEW_IS_COUNT_AT_DEPTH
#define DEBUG_VIEW_IS_COUNT_AT_ALL_DEPTHS
//#define DEBUG_VIEW_IS_CLUSTER_VALIDATION

#include "./../Common.hlsli"
#include "./../HiZCommon.hlsli"
#include "./Lights.hlsli"
#include "./HybridLightingCommon.hlsli"

StructuredBuffer<StructuredLight> lights : register(t0);
Texture2D<float> DepthRT : register(t3);
Texture2D<float2> HiZBuffer : register(t4);
RWStructuredBuffer<uint> ClusteredIndices : register(u0);
RWTexture2D<float4> DebugOut : register(u1);

cbuffer ClusterLight_CB : register(b4)
{
    uint4 _ClusterGroupResolutions; // groupsX, groupsY, groupsZ, padding
};

[numthreads(4, 4, 4)]
//[numthreads(1, 1, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    if (any(tId.xyz >= _ClusterGroupResolutions.xyz))
        return;
    
    // This one is for debug
    float3 idPerc = (float3) tId / _ClusterGroupResolutions.xyz;
    uint zSlice = tId.z;
    
    // Construct AABB
    float zNear = GetClusterZNear(zSlice);
    float zFar = GetClusterZNear(zSlice + 1u);
    
    bool isOccluded = false;
#if defined(CLUSTER_USE_FRUSTUM_TILES)
    // Since clusters match tiles with this method, we can test if every pixel is occluded
    float tileMaxDepth = HZB_LINEAR(HiZBuffer.Load(int3(tId.xy >> 4u, 4u)).g, _ZBufferParams);
    isOccluded = (tileMaxDepth < zNear);
#endif
    
    uint clusterIdx = GetClusterIdx(_ClusterGroupResolutions.xyz, tId);
    uint clusterDataIdx = MAX_LIGHTS_PER_CLUSTER_PLUS * clusterIdx;
    
    // Set light count to zero
    ClusteredIndices[clusterDataIdx] = 0u;
    uint nextLightIdx = 0u;
    
    // Calculate NDC for cluster corners
#if defined(CLUSTER_USE_AABB_TILES)
    float4 planeNDC = ((float4) (tId.xyxy * CLUSTER_DIMENSION + float4(CLUSTER_DIMENSION, CLUSTER_DIMENSION, 0.f, 0.f)) * _ScreenParams.zwzw) * 2.f - 1.f;
    planeNDC.yw *= -1.f;
#elif defined(CLUSTER_USE_FRUSTUM_TILES)
    float4 planeNDC = ((float4) (tId.xyxy * CLUSTER_DIMENSION + float4(CLUSTER_DIMENSION, 0.f, 0.f, CLUSTER_DIMENSION)) * _ScreenParams.zwzw) * 2.f - 1.f;
#endif
    
#if defined(CLUSTER_USE_AABB_TILES)
    AABB aabb = GetConservativeFrustumAABBFromNDC(planeNDC, zNear, zFar);
#elif defined(CLUSTER_USE_FRUSTUM_TILES)
    AABB aabb = GetFrustumAABBFromNDC(planeNDC, zNear, zFar);
    
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
    
    [branch]
    if (!isOccluded)
    {
        // Find lights in cluster
        [loop]
        for (uint i = 0u; i < _VisibleLightCount; ++i)
        {
            if (nextLightIdx >= MAX_LIGHTS_PER_CLUSTER)
                break;
            StructuredLight light = lights[i];
        
            // Calculate sphere to test against
            SphereBounds sphereBounds = GetSphereBoundsFromLight(light);
            
            bool inFrustum = true;
            
        #if defined(CLUSTER_USE_FRUSTUM_TILES)
            [unroll]
            for (uint j = 0u; j < 4u; ++j)
            {
                float d = dot(frustumPlanes[j], sphereBounds.positionVS.xyz);
                inFrustum = inFrustum && (d < sphereBounds.radius);
            }
        #endif
            
            inFrustum = inFrustum && AABBSphereIntersection(sphereBounds.positionVS, sphereBounds.radius, aabb.centerVS, aabb.extentsVS);
            
            inFrustum = inFrustum || (light.data0.x == 2); // always show directional lights
        
            [branch]
            if (inFrustum)
            {
                ClusteredIndices[clusterDataIdx + nextLightIdx + 1u] = i;
                nextLightIdx++;
            }
        }
    }
    // Set light count
    ClusteredIndices[clusterDataIdx] = nextLightIdx;

#if defined(ENABLE_DEBUG_VIEWS)
#if defined(DEBUG_VIEW_IS_COUNT_AT_DEPTH)
    // Show light count for one depth
    const uint depthLevel = 4u;
    float lightCountValue = (float) nextLightIdx / MAX_LIGHTS_PER_CLUSTER;
    if (tId.z == depthLevel)
    {
        for (uint x = 0; x < 16u; ++x)
        {
            for (uint y = 0; y < 16u; ++y)
            {
                uint2 pixelId = tId.xy * 16u + uint2(x, y);
                float greyscale = Linear01Depth(HiZBuffer.Load(int3(pixelId, 0u)).r, _ZBufferParams);
                greyscale *= greyscale;
                DebugOut[tId.xy * 16u + uint2(x, y)] = greyscale + lightCountValue;
            }
        }
    }
#elif defined (DEBUG_VIEW_IS_COUNT_AT_ALL_DEPTHS)
    // Each column represents light count at a depth level
    
    for (uint y = 0; y < 16u; ++y)
    {
        uint y2 = 16u - y;
        uint2 pixelId = tId.xy * 16u + uint2(tId.z, y2);
        uint2 outId = tId.xy * 16u + uint2(tId.z, y2);
        float greyscale = Linear01Depth(HiZBuffer.Load(int3(pixelId, 0u)).r, _ZBufferParams);
        greyscale *= greyscale;
        DebugOut[outId] = (y <= nextLightIdx) ? 1.f : greyscale;
        //DebugOut[outId] = saturate(-planeNDC.y);
        // Draw grid
        if (y == 0u || tId.z == 0u)
        {
            DebugOut[outId] = 1.f;
        }
    }
#elif defined(DEBUG_VIEW_IS_CLUSTER_VALIDATION)
    // Validate
    float2 screenUV = (tId.xy * CLUSTER_DIMENSION + CLUSTER_DIMENSION * 0.5f) * _ScreenParams.zw;
    float3 positionNDC = float3(screenUV * 2.f - 1.f, 0.f) * float3(1.f, -1.f, 1.f);
    float linearDepth = lerp(zNear, zFar, 0.01f);
    uint3 cluster = GetClusterFromNDC(positionNDC, linearDepth);

    bool isValid = all(cluster.xyz == tId.xyz);
    
    for (uint y = 0; y < 16u; ++y)
    {
        uint y2 = 16u - y;
        uint2 outId = tId.xy * 16u + uint2(tId.z, y2);
        DebugOut[outId] = isValid ? float4(0, 1, 0, 1) : float4(1, 0, 0, 1);
        DebugOut[outId] = saturate(positionNDC.y);
        // Draw grid
        if (y == 0u || tId.z == 0u)
        {
            DebugOut[outId] = 1.f;
        }
    }
#endif
#endif
}