#include "./CbufCommon.hlsli"
#include "./Lighting/LightStructs.hlsli"

#define MAX_TILE_LIGHTS 256
#define TILED_GROUP_SIZE 16

StructuredBuffer<StructuredLight> lights : register(t0);
Texture2D<float4> NormalRoughRT : register(t1);
Texture2D<float4> GBuffer1RT : register(t2);
StructuredBuffer<float> debug : register(t7);
RWTexture2D<float4> SpecularLightingOut : register(u0);
RWTexture2D<float4> DiffuseLightingOut : register(u1);

groupshared uint tileLightCount;
groupshared uint tileLightIndices[MAX_TILE_LIGHTS];

[numthreads(16, 16, 1)]
void CSMain(uint3 gId : SV_GroupID, uint gIndex : SV_GroupIndex, uint3 tId : SV_DispatchThreadID)
{
    //
    // Cull lights
    //
    
    // todo: use gId to setup frustums
    
    for (uint i = gIndex * TILED_GROUP_SIZE; i < gIndex * TILED_GROUP_SIZE + TILED_GROUP_SIZE, i < _VisibleLightCount; ++i)
    {
        StructuredLight light = lights[i];
        
        bool isVisible = true;

        [branch]
        if (isVisible)
        {
            uint idx;
            InterlockedAdd(tileLightCount, 1, idx);
            tileLightIndices[idx] = i;
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    //
    // Apply lighting
    //
    
    float vv = 0;
    for (uint i = 0; i < tileLightCount; ++i)
    {
        StructuredLight light = lights[tileLightIndices[i]];
        vv += lights[i].positionVS.x;
    }
    
    float4 normalRough = NormalRoughRT[tId.xy];
    float4 gbuff1 = GBuffer1RT[tId.xy];
    //ColorOut[tId.xy] = 1.f;
    //ColorOut[tId.xy] = NormalRoughRT[tId.xy].y;
    
    // todo: iterate through light list of tile and execute BRDF
    
    SpecularLightingOut[tId.xy] = normalRough.y * 0.01 + gbuff1.y * 0.01 + debug[0] * 0.01;
    DiffuseLightingOut[tId.xy] = 0.1f * 0;
}