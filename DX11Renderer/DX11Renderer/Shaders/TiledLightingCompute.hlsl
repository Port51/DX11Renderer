#include "./CbufCommon.hlsli"
#include "./Lighting/LightStructs.hlsli"

#define MAX_TILE_LIGHTS 64
#define TILED_GROUP_SIZE 4

StructuredBuffer<StructuredLight> lights : register(t0);
Texture2D<float4> NormalRoughRT : register(t1);
//Texture2D<float4> GBuffer1RT : register(t2);
Texture2D<float> DepthRT : register(t2);
StructuredBuffer<float> debug : register(t7);
RWTexture2D<float4> SpecularLightingOut : register(u0);
RWTexture2D<float4> DiffuseLightingOut : register(u1);
RWTexture2D<float4> DebugOut : register(u2);

groupshared uint tileLightCount;
groupshared uint tileLightIndices[MAX_TILE_LIGHTS];

/*float ConvertZToLinearDepth(float depth)
{
    float near = camera_near;
    float far = camera_far;
    
    return (near * far) / (far - depth * (far - near));

}

float3 GetPositionVS(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, inverse_projection);
    return homogenousLocation.xyz / homogenousLocation.w;
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

[numthreads(16, 16, 1)]
void CSMain(uint3 gId : SV_GroupID, uint gIndex : SV_GroupIndex, uint3 groupThreadId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
    //
    // Cull lights
    //
    
    if (gIndex == 0)
    {
        tileLightCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    // todo: use gId to setup frustums
    
    for (uint i = gIndex; i < _VisibleLightCount; i += TILED_GROUP_SIZE * TILED_GROUP_SIZE)
    {
        StructuredLight light = lights[i];
        
        bool isVisible = true;

        [branch]
        if (isVisible)
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
    float2 positionNDC = screenUV * 2.0 - 1.0;
    float rawDepth = DepthRT.Load(int3(tId.xy, 0));
    float linearDepth = RawDepthToLinearDepth(rawDepth);
    
    float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1);
    float3 positionVS = frustumPlaneVS * linearDepth;
    float3 viewDirVS = normalize(positionVS);
    
    
    
    //float view_depth = ConvertZToLinearDepth(depth);
    
    //float3 positionVS = GetPositionVS(uv, depth);
    //float3 viewDirVS = normalize(positionVS);
    float4 normalRough = NormalRoughRT[tId.xy];
    //float4 gbuff1 = GBuffer1RT[tId.xy];
    float3 normalVS = normalize(normalRough.xyz * 2.0 - 1.0);
    
    float3 diffuseLight = 0;
    float3 specularLight = 0;
    for (uint i = 0; i < 1; ++i) // tileLightCount
    {
        StructuredLight light = lights[tileLightIndices[i]];
        //StructuredLight light = lights[0];
        
	    // Simple lambert
        //float3 displ = mul(_ViewProj, float4(light.positionVS_Range.xyz, 1.0f)).xyz - positionVS.xyz;
        float3 displ = light.positionVS_Range.xyz - positionVS.xyz;
        
        float3 dir = normalize(displ);
        float NdotL = dot(normalVS, dir);
        
        //diffuseLight = 1.0 / length(displ);
        //diffuseLight = length(displ) < 5;
        //diffuseLight = (light.positionVS_Range.z < positionVS.z) * (rawDepth < 1);
        //diffuseLight = (displ.x > 0) * (rawDepth < 1);
        
        diffuseLight = saturate(NdotL) / (1.0 + dot(displ, displ));
        //diffuseLight = abs(normalVS.z);
        //diffuseLight = length(displ) * 0.1;
    }
    
    // Calibration
    //diffuseLight = positionVS.x < 5;
    //diffuseLight = diffuseLight * 0.5 + 0.5;
    diffuseLight *= (rawDepth < 1);
    //diffuseLight = positionNDC.y;
    
    //ColorOut[tId.xy] = 1.f;
    //ColorOut[tId.xy] = NormalRoughRT[tId.xy].y;
    
    // todo: iterate through light list of tile and execute BRDF
    
    SpecularLightingOut[tId.xy] = float4(specularLight, 1);
    DiffuseLightingOut[tId.xy] = float4(diffuseLight, 1);
    DebugOut[tId.xy] = float4(diffuseLight, 1);
    //DebugOut[tId.xy] = positionVS.x > 0;
    //DebugOut[tId.xy] = rawDepth < 1;
    //DebugOut[tId.xy] = rawDepth;
    //DebugOut[tId.xy] = viewDirVS.y;

}