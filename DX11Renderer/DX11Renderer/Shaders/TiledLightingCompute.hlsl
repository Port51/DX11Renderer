#include "./CbufCommon.hlsli"
#include "./Lighting/LightStructs.hlsli"

#define MAX_TILE_LIGHTS 64
#define TILED_GROUP_SIZE 16

#define DEBUG_GRID
#define DEBUG_LIGHT_RANGES

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
groupshared uint minTileZ;
groupshared uint maxTileZ;

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

[numthreads(TILED_GROUP_SIZE, TILED_GROUP_SIZE, 1)]
void CSMain(uint3 gId : SV_GroupID, uint gIndex : SV_GroupIndex, uint3 groupThreadId : SV_GroupThreadID, uint3 tId : SV_DispatchThreadID)
{
    //
    // Cull lights
    //
    
    if (gIndex == 0)
    {
        tileLightCount = 0;
        minTileZ = asuint(10000.0);
        maxTileZ = asuint(0.0);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // todo: replace w/ Hi-Z buffer
    float rawDepth = DepthRT.Load(int3(tId.xy, 0));
    float linearDepth = RawDepthToLinearDepth(rawDepth);
    uint intDepth = asuint(linearDepth);
    InterlockedMin(minTileZ, intDepth);
    InterlockedMax(maxTileZ, intDepth);
    
    GroupMemoryBarrierWithGroupSync();
    
    float2 tileDepthRange = float2(asfloat(minTileZ), asfloat(maxTileZ));
    
    // todo: use gId to setup frustums
    // todo: early out if only sky
    
    float debugValue = 0;

    // Derive frustum planes
    // #0 faces to the right, and the rest continue in CCW fashion
    float3 frustumPlanes[4];
    
    // Calculate NDC for all sides
    float4 planeNDC = ((float4) (gId.xyxy * TILED_GROUP_SIZE + float4(TILED_GROUP_SIZE, 0, 0, TILED_GROUP_SIZE)) * _ScreenParams.zwzw) * 2.0 - 1.0;
    // Use cross product to turn tile view directions into plane directions
    // The cross product is done by flipping X or Y with Z
    frustumPlanes[0] = normalize(float3(1, 0, -planeNDC.x * _FrustumCornerDataVS.x));
    frustumPlanes[1] = normalize(float3(0, 1, -planeNDC.y * _FrustumCornerDataVS.y));
    frustumPlanes[2] = normalize(float3(-1, 0, planeNDC.z * _FrustumCornerDataVS.x));
    frustumPlanes[3] = normalize(float3(0, -1, planeNDC.w * _FrustumCornerDataVS.y));
    
    float3 testDirGroup = float3(planeNDC.zy * _FrustumCornerDataVS.xy, 1);
    float3 testDirThread = float3((tId.xy * _ScreenParams.zw * 2.0 - 1.0) * _FrustumCornerDataVS.xy, 1);
    float3 dp = testDirThread - testDirGroup;
    
    //debugValue = frustumPlanes[0].x * 10 - 9;
    //debugValue = frustumPlanes[0].z;
    //debugValue = abs(frustumPlanes[1].z) < 0.001;
    //debugValue = dot(lights[1].positionVS_Range.xyz, -frustumPlanes[2]);
    //debugValue = (frustumPlanes[0].z + frustumPlanes[2].z) * 10;
    //debugValue = planeNDC.x > planeNDC.z;
    
    for (uint i = gIndex; i < _VisibleLightCount; i += TILED_GROUP_SIZE * TILED_GROUP_SIZE)
    {
        StructuredLight light = lights[i];
        
        bool inFrustum = true;
        
        if (true) // todo: test light type
        {
            [unroll]
            for (uint i = 0; i < 4; ++i)
            {
                float d = dot(frustumPlanes[i], light.positionVS_Range.xyz);
                inFrustum = inFrustum && (d < light.positionVS_Range.w);
            }
            inFrustum = inFrustum && (light.positionVS_Range.z >= tileDepthRange.x - light.positionVS_Range.w);
            inFrustum = inFrustum && (light.positionVS_Range.z <= tileDepthRange.y + light.positionVS_Range.w);
        }
        
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
    float2 positionNDC = screenUV * 2.0 - 1.0;
    
    float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1);
    float3 positionVS = frustumPlaneVS * linearDepth;
    float3 viewDirVS = normalize(positionVS);
    
    float4 normalRough = NormalRoughRT[tId.xy];
    //float4 gbuff1 = GBuffer1RT[tId.xy];
    float3 normalVS = normalize(normalRough.xyz * 2.0 - 1.0);
    
    float3 diffuseLight = 0;
    float3 specularLight = 0;
    for (uint i = 0; i < tileLightCount; ++i)
    {
        StructuredLight light = lights[tileLightIndices[i]];
        
	    // Simple lambert
        float3 displ = light.positionVS_Range.xyz - positionVS.xyz;
        
        float3 dir = normalize(displ);
        float NdotL = dot(normalVS, dir);
        
        // todo: special pt light attenuation
        //diffuseLight += light.color.rgb * (light.intensity * saturate(NdotL) / (1.0 + dot(displ, displ)));
        diffuseLight += light.color.rgb * (1.0 - length(displ) / light.positionVS_Range.w);
    }
    
    diffuseLight *= (rawDepth < 1);
    specularLight *= (rawDepth < 1);
    
    // todo: iterate through light list of tile and execute BRDF
    float3 debugColor = debugValue;// * (rawDepth < 1);
    
#if defined(DEBUG_LIGHT_RANGES)
    debugColor = float3(diffuseLight.r * (rawDepth < 1), tileLightCount * 0.33, 0);
#endif
    
#if defined(DEBUG_GRID)
    if (groupThreadId.x == 0 || groupThreadId.y == 0)
    {
        debugColor = float3(0, 1, 0);
    }
#endif
    //debugColor = saturate(asfloat(maxTileZ) - asfloat(minTileZ));
    //debugColor = asfloat(minTileZ) / 20.0;
    
    SpecularLightingOut[tId.xy] = float4(specularLight, 1);
    DiffuseLightingOut[tId.xy] = float4(diffuseLight, 1);
    //DebugOut[tId.xy] = float4(lerp(float3(1, 1, 1), float3(1, 0, 0), tileLightCount * 0.333) * (rawDepth < 1), 1);
    DebugOut[tId.xy] = float4(debugColor, 1);
    //DebugOut[tId.xy] = float4(diffuseLight, 1);
    //DebugOut[tId.xy] = sides[0].y;
    //DebugOut[tId.xy] = positionVS.x > 0;
    //DebugOut[tId.xy] = rawDepth < 1;
    //DebugOut[tId.xy] = rawDepth;
    //DebugOut[tId.xy] = viewDirVS.y;

}