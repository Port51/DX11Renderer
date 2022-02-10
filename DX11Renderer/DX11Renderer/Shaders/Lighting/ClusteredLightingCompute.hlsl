// References:
//  - http://www.aortiz.me/2018/12/21/CG.html#part-2

#define ENABLE_DEBUG_VIEWS // much faster if turned off
//#define DEBUG_VIEW_IS_COUNT_AT_DEPTH
#define DEBUG_VIEW_IS_COUNT_AT_ALL_DEPTHS

#include "./../Common.hlsli"
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
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    uint zSlice = tId.z;
    
    // Construct AABB
    float zNear = GetClusterZNear(zSlice);
    float zFar = GetClusterZNear(zSlice + 1u);
    
    // Get max depth
    float2 tileDepthRange = HiZBuffer.Load(int3(tId.xy >> 4u, 4u)).rg;
    tileDepthRange.x = Linear01Depth(tileDepthRange.x, _ZBufferParams);
    tileDepthRange.y = Linear01Depth(tileDepthRange.y, _ZBufferParams);
    
    // Early out if 100% occluded (is this relevant with conservative AABBs?)
    bool occluded = (tileDepthRange.y > zFar);
    occluded = false;
    
    uint clusterIdx = GetClusterIdx(_ClusterGroupResolutions.xyz, tId);
    uint clusterDataIdx = MAX_LIGHTS_PER_CLUSTER_PLUS * clusterIdx;
    
    // Set light count to zero
    ClusteredIndices[clusterDataIdx] = 0u;
    uint nextLightIdx = 0u;
    
    if (!occluded)
    {
        // Calculate NDC for cluster corners
        const float4 planeNDC = ((float4) (tId.xyxy * CLUSTER_DIMENSION + float4(CLUSTER_DIMENSION, 0.f, 0.f, CLUSTER_DIMENSION)) * _ScreenParams.zwzw) * 2.f - 1.f;
        AABB aabb = GetConservativeFrustumAABBFromNDC(planeNDC, zNear, zFar);
        for (uint i = 0u; i < _VisibleLightCount; ++i)
        {
            StructuredLight light = lights[i];
        
            // Calculate sphere to test against
            SphereBounds sphereBounds = GetSphereBoundsFromLight(light);
        
            [branch]
            if (AABBSphereIntersection(sphereBounds.positionVS, sphereBounds.radius, aabb.centerVS, aabb.extentsVS))
            {
                ClusteredIndices[clusterDataIdx + nextLightIdx + 1u] = nextLightIdx;
                nextLightIdx++;
                ClusteredIndices[clusterDataIdx] = nextLightIdx;
            }
        }
    }

#if defined(ENABLE_DEBUG_VIEWS)
    #if defined(DEBUG_VIEW_IS_COUNT_AT_DEPTH)
    // Show light count for one depth
    const uint depthLevel = 1u;
    float lightCountValue = (float) nextLightIdx / MAX_LIGHTS_PER_CLUSTER;
    if (tId.z == depthLevel)
    {
        for (uint x = 0; x < 16u; ++x)
        {
            for (uint y = 0; y < 16u; ++y)
            {
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
        float greyscale = Linear01Depth(HiZBuffer.Load(int3(pixelId, 0u)).r, _ZBufferParams);
        greyscale *= greyscale;
        DebugOut[tId.xy * 16u + uint2(tId.z, y2)] = (y * 2u <= nextLightIdx) ? 1.f : greyscale;
        // Draw grid
        if (y == 0u || tId.z == 0u)
        {
            DebugOut[tId.xy * 16u + uint2(tId.z, y2)] = 1.f;
        }
    }
    #endif
#endif
}