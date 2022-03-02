// References:
//  - https://www.jpgrenier.org/ssr.html
//  - https://sakibsaikia.github.io/graphics/2016/12/26/Screen-Space-Reflection-in-Killing-Floor-2.html

#include "./Common.hlsli"
#include "./HiZCommon.hlsli"

// Set SSR method here
//#define SSR_METHOD_3D_RAYMARCH
#define SSR_METHOD_NC_DDA
//#define SSR_METHOD_BINARY_SEARCH
//#define SSR_METHOD_HI_Z
//#define SSR_METHOD_CONE_TRACE

#define MAX_MIP                 7u
#define MAX_TRACE_ITERATIONS    20u

#define DEBUG_VIEW_SHOW_TRACE

Texture2D<float4> NormalRoughReflectivityRT : register(t2);
Texture2D<float4> CameraColorIn : register(t3);
Texture2D<float> DepthRT : register(t4);
Texture2D<float2> HiZBuffer : register(t5);

RWTexture2D<float4> CameraColorOut : register(u0);
globallycoherent RWStructuredBuffer<uint> DebugData : register(u1);
RWTexture2D<float4> DebugOut : register(u2);

groupshared uint2 debugTraceId;

cbuffer SSR_CB : register(b4)
{
    uint _DebugViewStep;
    uint3 padding;
};

float4 PositionVSToSS(float3 positionVS)
{
    float4 positionSS = mul(_ProjMatrix, float4(positionVS, 1.f));
    positionSS.xyz /= positionSS.w;
    positionSS.xy = (positionSS.xy * float2(0.5f, -0.5f) + 0.5f) * _ScreenParams.xy;
    return positionSS; // return w component as well for linear depth
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
float GetPerspectiveCorrectDepth(float d0, float d1, float lerpValue)
{
    // todo: optimize!
    // EQ: Z = 1 / [ 1/Z0 (1 - lerp) + 1/Z1 * lerp ]
    return rcp(lerp(rcp(d0), rcp(d1), lerpValue));
}

// ref: https://jhstrom.blogspot.com/2021/03/screen-space-reflections-explained.html
float3 GetReflectionDirSS(float3 positionVS, float3 reflectDirVS, float2 uv, float rawDepth)
{
    float3 positionSS = float3(uv, rawDepth); // screen pixels, and non-linear depth
    float3 offsetPositionSS = PositionVSToSS(positionVS + reflectDirVS);
    float3 initialDir = normalize(offsetPositionSS - positionSS);
    
    // Ensure each axis is not too close to zero
    float3 absDir = abs(initialDir);
    float ep = 0.0001f;
    return float3(
        absDir.x < ep ? ep : initialDir.x,
        absDir.y < ep ? ep : initialDir.y,
        absDir.z < ep ? ep : initialDir.z);
    
    /*const float3 positionSS = float3(uv.xy, rawDepth);
    
    const float Offset = 0.5f;
    const float3 offsetPositionVS = positionVS + reflectDirVS * Offset;
    float4 offsetPositionSS = mul(_ProjMatrix, float4(offsetPositionVS, 0.f));
    offsetPositionSS.xyz /= offsetPositionSS.w;
    offsetPositionSS.xy = (offsetPositionSS.xy * float2(0.5f, -0.5f) + 0.5f) * _ScreenParams.xy;

    return (offsetPositionSS.xyz - positionSS);*/
}

// A simple 3D raymarch that oversamples as rays get farther from the camera
// It's way better to search in screen-space
float4 GetReflectColor_3DRaymarch(uint3 tId, float3 positionVS, float3 reflectDirVS)
{
    const bool isDebugTrace = (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y);
    
    const uint precision = 10u;
    const uint iterations = MAX_TRACE_ITERATIONS * precision;
    const float maxRayDist = 25.f;
    const float thickness = 0.1f;
    const float dt = maxRayDist / (float)iterations;
    
    float4 lastTraceSS = PositionVSToSS(positionVS);
    for (uint i = 0u; i < iterations; ++i)
    {
        float3 traceVS = positionVS + (dt * i + dt) * reflectDirVS;
        float4 traceSS = PositionVSToSS(traceVS);
        
#if defined(DEBUG_VIEW_SHOW_TRACE)
        if (isDebugTrace && (i / precision) == _DebugViewStep)
        {
            DebugData[0] = traceSS.x;
            DebugData[1] = traceSS.y;
            DebugData[2] = 0u;
        }
#endif
        
        // Read depth and convert it to linear depth
        float depth = HiZBuffer.Load(int3(traceSS.xy, 0)).x;
#if !defined(HZB_USES_LINEAR_DEPTH)
        depth = LinearEyeDepth(depth, _ZBufferParams);
#else
        depth = Depth01ToEyeDepth(depth);
#endif
        
        // Test depth intersection
        float minTraceZ = min(traceSS.w, lastTraceSS.w);
        float maxTraceZ = max(traceSS.w, lastTraceSS.w);
        if (maxTraceZ > depth && minTraceZ < depth + thickness)
        {
            float confidence = 1.f - pow((float)i / iterations, 2.f);
            //confidence = 1.f;
            
            // Ensure facing surface
            const float3 traceNormalVS = GetNormalVSFromGBuffer(NormalRoughReflectivityRT[traceSS.xy]);
            float NdotL = saturate(dot(reflectDirVS, traceNormalVS) * -5.f);
            confidence *= NdotL;
            
#if defined(DEBUG_VIEW_SHOW_TRACE)
            if (isDebugTrace && (i / precision) < _DebugViewStep)
            {
                DebugData[0] = traceSS.x;
                DebugData[1] = traceSS.y;
                DebugData[2] = 0u;
            }
#endif
            
            return float4(CameraColorIn[traceSS.xy].rgb, confidence * 0.5f);
        }
    }
    return 0.f;
}

float4 GetReflectColor_NC_DDA(uint3 tId, float3 positionVS, float4 positionSS, float3 reflectDirVS)
{
    const bool isDebugTrace = (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y);
    
    const float maxRayLength = 25.f;
    const float stride = 1.f;
    const float thickness = 0.1f;
    
    // Clip to near plane to avoid possible division by 0 during perspective divide
    float rayLength = ((positionVS.z + reflectDirVS.z * maxRayLength) < _ProjectionParams.y) ? (_ProjectionParams.y - positionVS.z) / reflectDirVS.z : maxRayLength;
    
    positionSS = PositionVSToSS(positionVS);
    float3 endPtVS = positionVS + reflectDirVS * rayLength;
    float4 endPtSS = PositionVSToSS(endPtVS);
    
    // Handle case when endpt is very close
    endPtSS += dot(endPtSS.xy - positionSS.xy, endPtSS.xy - positionSS.xy) < 0.0001f ? 0.01f : 0.f;
    
    float2 delta = endPtSS.xy - positionSS.xy;
    
    // Decide whether to swap X and Y
    bool swapXY = false;
    if (abs(delta.x) < abs(delta.y))
    {
        swapXY = true;
        delta = delta.yx;
        positionSS.xy = positionSS.yx;
        endPtSS.xy = endPtSS.yx;
    }
    
    float stepDir = sign(delta.x);
    float invDx = stepDir / delta.x;
    
    float2 dSS = float2(stepDir, delta.y * invDx) * stride;
    
    float jitter = 0.01f; // todo: use RNG
    float tracePts = abs(endPtSS.x - positionSS.x);
    float invQ = stride / tracePts;
    
    float prevTraceZ = GetPerspectiveCorrectDepth(positionVS.z, endPtVS.z, invQ); // offset by 1 pixel
    
    // Debug
    uint sampleSpread = ceil(tracePts / MAX_TRACE_ITERATIONS);
    
    // Offset
    positionSS.xy += dSS;
    
    for (float i = 2.f; i <= tracePts; i += 1.f)
    {
        float2 traceSS = positionSS.xy + dSS * i;
        
        float2 unswappedSS = (swapXY) ? traceSS.yx : traceSS.xy;
        float traceZ = GetPerspectiveCorrectDepth(positionVS.z, endPtVS.z, i * invQ);
        
        // Read depth and convert it to linear depth
        float depth = HiZBuffer.Load(int3(unswappedSS.xy, 0)).x;
        if (depth == 0.f)
            break;
#if !defined(HZB_USES_LINEAR_DEPTH)
        depth = LinearEyeDepth(depth, _ZBufferParams);
#else
        depth = Depth01ToEyeDepth(depth);
#endif
        
#if defined(DEBUG_VIEW_SHOW_TRACE)
        if (isDebugTrace && i == _DebugViewStep * sampleSpread)
        {
            DebugData[0] = unswappedSS.x;
            DebugData[1] = unswappedSS.y;
            DebugData[2] = 0u;
        }
#endif
        
        // Test depth intersection
        float minTraceZ = min(traceZ, prevTraceZ);
        float maxTraceZ = max(traceZ, prevTraceZ);
        if ((maxTraceZ > depth && minTraceZ < depth + thickness) || i == 1000)
        {
            float confidence = 1.f;
            
            // Ensure facing surface
            const float3 traceNormalVS = GetNormalVSFromGBuffer(NormalRoughReflectivityRT[unswappedSS.xy]);
            float NdotL = saturate(dot(reflectDirVS, traceNormalVS) * -5.f);
            confidence *= NdotL;
            
#if defined(DEBUG_VIEW_SHOW_TRACE)
            if (isDebugTrace && i < _DebugViewStep * sampleSpread)
            {
                DebugData[0] = unswappedSS.x;
                DebugData[1] = unswappedSS.y;
                DebugData[2] = 0u;
            }
#endif
            
            return float4(CameraColorIn[unswappedSS.xy].rgb, confidence * 0.5f);
        }
        
        prevTraceZ = traceZ;

    }
    return 0.f;
}

float4 GetReflectColor_SimpleSearch(uint3 tId, float rawDepth, float3 reflectDirSS)
{
    const bool isDebugTrace = (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y);
    
    reflectDirSS = normalize(reflectDirSS);
    
    float3 uv = float3(tId.xy + 0.5f, rawDepth - 0.001f);
    const uint iterations = MAX_TRACE_ITERATIONS * 5u;
    for (uint i = 0u; i < iterations; ++i)
    {
#if defined(DEBUG_VIEW_SHOW_TRACE)
        if (isDebugTrace && (i / 5u) == _DebugViewStep)
        {
            DebugData[0] = uv.x;
            DebugData[1] = uv.y;
            DebugData[2] = 0u;
        }
#endif
        
        uv += reflectDirSS * 0.1f;
        
        // Test depth intersection
        float depth = HiZBuffer.Load(int3(uv.xy, 0)).x;
        if (uv.z > depth)
        {
            return float4(CameraColorIn[uv.xy].rgb, 1.f - (float)i / iterations);
        }
    }
    return 0.f;
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    //if (tId.x >= (uint) _ScreenParams.x || tId.y >= (uint) _ScreenParams.y)
    //    return;
    
    // todo: determine if out of screen bounds if depth == 0 (accessing texture outside bounds returns 0)
    // todo: get hyperbolic depth from p.z / p.w, and get linear depth from p.w
    // todo: clip ray end pt against near plane for rays facing camera (to avoid division by 0 during perspective divide)
    
    //
    // Setup debug views
    //
    const bool isDebugTrace = (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y);
    
    //
    // Get surface info
    //
    const float rawDepth = DepthRT.Load(int3(tId.xy, 0));
    const float isGeometry = rawDepth < 1.f;
    const float linearDepth = RawDepthToLinearDepth(rawDepth);
    
    //const float linearDepth = HiZBuffer.Load(int3(tId.xy, 0)).r * _ProjectionParams.z;
    const float2 screenUV = tId.xy * _ScreenParams.zw;
    const float2 positionNDC = screenUV * 2.f - 1.f;
    
    const float3 frustumPlaneVS = float3(positionNDC * _FrustumCornerDataVS.xy, 1.f);
    const float3 positionVS = frustumPlaneVS * linearDepth;
    const float4 positionSS = float4(tId.xy, 0.f, linearDepth); // not using parabolic depth right now...
    const float3 viewDirVS = normalize(positionVS);
    
    // VERIFICATION:
    //float3 fakePosSS = PositionVSToSS(positionVS);
    //float error = abs(fakePosSS.z - rawDepth) * 1000.0f;
    //DebugOut[tId.xy] = error;
    //CameraColorOut[tId.xy] = error;
    //return;
    
    const float4 gbufferTex = NormalRoughReflectivityRT[tId.xy];
    const float3 normalVS = GetNormalVSFromGBuffer(gbufferTex);
    
    const float linearRoughness = gbufferTex.z;
    const float roughness = linearRoughness * linearRoughness;
    const float reflectivity = gbufferTex.w;
    
    //
    // Calculate reflection
    //
    const float3 reflectDirVS = normalize(reflect(viewDirVS, normalVS));
    const float3 reflectDirSS = GetReflectionDirSS(positionVS, reflectDirVS, tId.xy, rawDepth);
    const float3 invDir = 1.f / reflectDirSS;
    const float2 halfSignDir = sign(reflectDirSS.xy) * 0.5f;
    
    float pixelDist = min(abs(invDir.x), abs(invDir.y));
    float3 uv = float3(tId.xy + 0.5f, rawDepth) + reflectDirSS * pixelDist; // start from center of pixel, with 1-pixel offset
    float4 reflectColor = 0.f;
    uint iter = 0u;
    uint mip = 3u;
    
    /*[branch]
    if (reflectDirSS.z > 0.f)
    {
        while (iter < MAX_TRACE_ITERATIONS)
        {
            uint2 ptId = (uint2)floor(uv.xy);
            uint2 tileId = ptId >> mip;
            uint tilePixelSize = 1u << mip;
        
#if defined(DEBUG_VIEW_SHOW_TRACE)
            if (isDebugTrace && iter == _DebugViewStep)
            {
                DebugData[0] = ptId.x;
                DebugData[1] = ptId.y;
                DebugData[2] = mip;
            }
#endif
        
            // Setup planes
            float2 planesUV = ((tileId + 0.5f) * tilePixelSize + tilePixelSize * halfSignDir);
        
            // Calculate nearest intersection
            // todo: fix error when solutions are negative
            float2 intersectionSolutions = max(0.f, (planesUV - uv.xy) * invDir.xy);
            float intersectDist = min(intersectionSolutions.x, intersectionSolutions.y);
            float3 intersectUV;
        
            float tileIntersectDist;
            float2 tileIntersectOffset;
            if (intersectionSolutions.x < intersectionSolutions.y)
            {
                tileIntersectDist = intersectionSolutions.x;
                tileIntersectOffset = float2(halfSignDir.x, 0.f); // 1/2 pixel offset
            }
            else
            {
                tileIntersectDist = intersectionSolutions.y;
                tileIntersectOffset = float2(0.f, halfSignDir.y); // 1/2 pixel offset
            }
            
            // Calculate depth plane intersection
            float mipDepthPlane = HiZBuffer.Load(int3(tileId.xy, mip)).x;
            float mipDepthDispl = (mipDepthPlane - uv.z);
            float depthPlaneSolution = mipDepthDispl * invDir.z;
        
            // Test depth ranges
            if (depthPlaneSolution < tileIntersectDist)
            {
                if (mip == 0u)
                {
                    float3 hitNormalVS = GetNormalVSFromGBuffer(NormalRoughReflectivityRT[ptId.xy]);
                    
                    //reflectColor.rgb = CameraColorIn[ptId].rgb;
                    reflectColor.rgb = saturate(dot(reflectDirVS, hitNormalVS));
                    //reflectColor.a = saturate(dot(reflectDirVS, hitNormalVS) * -1.f);
                    reflectColor.a = 1;
                    break;
                }
                else
                {
                    --mip;
                    uv = reflectDirSS * depthPlaneSolution + uv;
                }
            }
            else
            {
                // Move to tile intersection and increase mip
                uv = reflectDirSS * tileIntersectDist + uv + float3(tileIntersectOffset.xy, 0.f);
                mip = min(mip + 1u, MAX_MIP);
            }
        
            ++iter;
        }
    }*/
    
#if defined(SSR_METHOD_3D_RAYMARCH)
    reflectColor = GetReflectColor_3DRaymarch(tId, positionVS, reflectDirVS);
#elif defined(SSR_METHOD_NC_DDA)
    reflectColor = GetReflectColor_NC_DDA(tId, positionVS, positionSS, reflectDirVS);
#endif
    
    // todo: use fallback when reflectColor.a = 0
    
    float4 colorIn = CameraColorIn[tId.xy];
    CameraColorOut[tId.xy] = float4(colorIn.rgb + reflectColor.rgb * (reflectColor.a * reflectivity), colorIn.a);
    
    //CameraColorOut[tId.xy] = reflectColor;
    //CameraColorOut[tId.xy] = saturate(reflectDirVS.z);
    
    //
    // Debug views!
    //    
#if defined(DEBUG_VIEW_SHOW_TRACE)
    // Show trace of current point
    
    AllMemoryBarrierWithGroupSync();
    
    float3 debugColor = saturate(CameraColorOut[tId.xy].rgb);
    
    if (isDebugTrace)
    {
        // Show start point
        debugColor = float3(1, 1, 0);
    }
    
    uint2 mipMod = tId.xy % (1u << DebugData[2]);
    if (tId.x == DebugData[0] && tId.y == DebugData[1])
    {
        debugColor = float3(0, 1, 0);
    }
    else if (tId.x == DebugData[0] || tId.y == DebugData[1])
    {
        debugColor = lerp(debugColor.rgb, float3(1, 0, 0), 0.8f);
    }
    else if (DebugData[2] > 0u && any(mipMod == 0u))
    {
        // Tiles
        debugColor = lerp(debugColor, float3(0, 1, 0), 0.15f);

    }
    
    DebugOut[tId.xy] = float4(debugColor.rgb, colorIn.a);
#endif
    
}