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

//#define SSR_USES_THICKNESS_TEST
#define SSR_THICKNESS           0.1f
#define SSR_ALLOWS_CAMERA_FACING_RAYS

#define MAX_MIP                 7u
#define MAX_TRACE_ITERATIONS    150u

#define DEBUG_VIEW_SHOW_TRACE

#if defined(DEBUG_VIEW_SHOW_TRACE)
    #define DEBUG_TRACE_PIXEL(initPt, pt, mip, iter) \
        if (all(initPt.xy == _PixelSelection.xy) && iter == _PixelSelection.z) \
        { \
            DebugData[0] = pt.x; \
            DebugData[1] = pt.y; \
            DebugData[2] = mip; \
        }
#endif

Texture2D<float4> NormalRoughReflectivityRT : register(t2);
Texture2D<float4> CameraColorIn : register(t3);
Texture2D<float> DepthRT : register(t4);
Texture2D<float2> HiZBuffer : register(t5);
Texture2D<float> DitherTex : register(t6);

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

// ref: https://jhstrom.blogspot.com/2021/03/screen-space-reflections-explained.html
float3 GetReflectionDirSS(float3 positionVS, float3 reflectDirVS, float2 uv, float rawDepth)
{
    float3 positionSS = float3(uv, rawDepth); // screen pixels, and non-linear depth
    float3 offsetPositionSS = PositionVSToSS(positionVS + reflectDirVS).xyz;
    float3 initialDir = normalize(offsetPositionSS - positionSS);
    
    // Ensure each axis is not too close to zero
    float3 absDir = abs(initialDir);
    float ep = 0.0001f;
    return float3(
        absDir.x < ep ? ep : initialDir.x,
        absDir.y < ep ? ep : initialDir.y,
        absDir.z < ep ? ep : initialDir.z);
}

// Same as GetReflectionDirSS(), but no normalization
// this is used for when you want to interpolate from start to end using a lerp value between 0 and 1
// Format: (x pixels, y pixels, z depth)
float3 GetReflectionDistanceSS(float3 positionVS, float3 reflectDirVS, float2 uv, float rayDistance)
{
    float3 positionSS = float3(uv.xy, positionVS.z);
    float4 offsetPositionSS = PositionVSToSS(positionVS + reflectDirVS * rayDistance);
    float3 initialDir = (offsetPositionSS.xyw - positionSS.xyz); // w component is intentional
    
    // Ensure each axis is not too close to zero
    float3 absDir = abs(initialDir);
    float ep = 0.0001f;
    return float3(
        absDir.x < ep ? ep : initialDir.x,
        absDir.y < ep ? ep : initialDir.y,
        absDir.y < ep ? ep : initialDir.z);
}

struct DDAParams
{
    bool swapXY;
    float2 startPosSS;
    float2 endPosSS;
    float2 dSS;
    float traceRange;
    float invTraceRange;
    float invStartDepth;
    float invEndDepth;
};

// Sets up all info required for a search using DDA line algorithm
// Includes some optimizations, like precalculating divisions so they are only done once
DDAParams GetDDAParams(float4 startPosSS, float4 endPosSS)
{
    DDAParams params;
    
    // Handle case when endpt is very close
    endPosSS += dot(startPosSS.xy - startPosSS.xy, endPosSS.xy - startPosSS.xy) < 0.0001f ? 0.01f : 0.f;
    
    float2 delta = endPosSS.xy - startPosSS.xy;
    
    // Decide whether to swap X and Y
    params.swapXY = false;
    if (abs(delta.x) < abs(delta.y))
    {
        params.swapXY = true;
        delta = delta.yx;
        startPosSS.xy = startPosSS.yx;
        endPosSS.xy = endPosSS.yx;
    }
    
    float stepDir = sign(delta.x);
    params.dSS = float2(stepDir, delta.y * stepDir / delta.x);
    
    params.traceRange = abs(endPosSS.x - startPosSS.x);
    params.invTraceRange = 1.f / params.traceRange;
    params.startPosSS = startPosSS.xy;
    params.endPosSS = endPosSS.xy;
    params.invStartDepth = 1.f / startPosSS.w;
    params.invEndDepth = 1.f / endPosSS.w;
    
    return params;

}

struct DDAPt
{
    float2 traceSS;
    float traceDepth;
    float sampleDepth;
};

// Samples a pt along a DDA line, using perspective correct depth interpolation
// Output: (x, y, trace-depth, sampled-depth)
DDAPt SampleDDAPt(DDAParams params, float steps, float jitter)
{
    DDAPt pt = (DDAPt)0.f;
    pt.sampleDepth = 0.f;
    steps = min(params.traceRange, steps); // clamp to farthest pt
    float2 traceSS = params.startPosSS.xy + params.dSS * steps;
        
    float2 unswappedSS = (params.swapXY) ? traceSS.yx : traceSS.xy;
    float traceZ = GetPerspectiveCorrectDepth_Optimized(params.invStartDepth, params.invEndDepth, steps * params.invTraceRange);
        
        // Read depth and convert it to linear depth
    float depth = HiZBuffer.Load(int3(unswappedSS.xy, 0)).x;
    if (depth == 0.f) // early return if sample out of viewport
        return pt;
    depth = HZB_LINEAR(depth, _ZBufferParams);
    depth += jitter; // to cut down on banding
    
    pt.traceSS = unswappedSS;
    pt.traceDepth = traceZ;
    pt.sampleDepth = depth;
    return pt;
}

// Helper function for executing raymarching using non-conservative DDA line algorithm
// Output: (x, y, travelDistance, confidence)
float4 CourseDDASearch(DDAParams params, float3 reflectDirVS, float stride, float jitter, bool isDebugTrace)
{
    // Debug
    const uint debugSampleSpread = ceil(params.traceRange / MAX_TRACE_ITERATIONS);
    //sampleSpread = 1; // uncomment to verify stride
    
    float prevTraceZ = 1.f / params.invStartDepth;
    
    for (float i = 1.f; i <= params.traceRange; i += stride)
    {
        DDAPt pt = SampleDDAPt(params, i, jitter);
        if (pt.sampleDepth == 0.f)
            break;
        
#if defined(DEBUG_VIEW_SHOW_TRACE)
        if (isDebugTrace && (uint)i == _DebugViewStep * debugSampleSpread)
        {
            DebugData[0] = pt.traceSS.x;
            DebugData[1] = pt.traceSS.y;
            DebugData[2] = 0u;
        }
#endif
        
        // Test depth intersection
        const float maxTraceZ = max(pt.traceDepth, prevTraceZ);
#if defined(SSR_USES_THICKNESS_TEST)
        if (maxTraceZ > pt.sampleDepth && maxTraceZ <= pt.sampleDepth + SSR_THICKNESS)
#else
        if (maxTraceZ > pt.sampleDepth)
#endif
        {
            float confidence = 1.f;
            
            // Ensure facing surface
            const float3 traceNormalVS = GetNormalVSFromGBuffer(NormalRoughReflectivityRT[pt.traceSS.xy]);
            float NdotL = saturate(dot(reflectDirVS, traceNormalVS) * -5.f);
            confidence *= NdotL;
            
#if defined(DEBUG_VIEW_SHOW_TRACE)
            if (isDebugTrace && (uint)i < _DebugViewStep * debugSampleSpread)
            {
                DebugData[0] = pt.traceSS.x;
                DebugData[1] = pt.traceSS.y;
                DebugData[2] = 0u;
            }
#endif
            
            return float4(pt.traceSS.xy, i, confidence * 0.5f);
        }
        
        prevTraceZ = pt.traceDepth;
    }
    return 0.f;
}

//
// A simple 3D raymarch that oversamples as rays get farther from the camera
// It's way better to search in screen-space
//
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
        float depth = HZB_LINEAR(HiZBuffer.Load(int3(traceSS.xy, 0)).x, _ZBufferParams);
        
        // Test depth intersection
        float minTraceZ = min(traceSS.w, lastTraceSS.w);
        float maxTraceZ = max(traceSS.w, lastTraceSS.w);
        if (maxTraceZ > depth && minTraceZ < depth + thickness)
        {
            float confidence = 1.f - pow((float)i / iterations, 2.f);
            
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

//
// Uses a non-conservative DDA line algorithm to search every pixel
// This is an improvement on the 3D search, as it searches once per pixel and
// doesn't over/under sample based on depth
//
float4 GetReflectColor_NC_DDA(uint3 tId, float3 positionVS, float4 positionSS, float3 reflectDirVS, float dither)
{
    const bool isDebugTrace = (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y);
    
    const float maxRayLength = 25.f;
    const float stride = 1.f;
    const float thickness = 0.1f;
    
    float jitter = dither * 0.0091f;
    
    // Clip to near plane to avoid possible division by 0 during perspective divide
    float rayLength = ((positionVS.z + reflectDirVS.z * maxRayLength) < _ProjectionParams.y) ? (_ProjectionParams.y - positionVS.z) / reflectDirVS.z : maxRayLength;
    
    positionSS = PositionVSToSS(positionVS);
    float3 endPtVS = positionVS + reflectDirVS * rayLength;
    float4 endPtSS = PositionVSToSS(endPtVS);
    
    // Precalculate these (optimization)
    float invStartPtDepth = 1.f / positionVS.z;
    float invEndPtDepth = 1.f / endPtVS.z;
    
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
    float2 dSS = float2(stepDir, delta.y * stepDir / delta.x);
    
    float traceRange = abs(endPtSS.x - positionSS.x);
    float invTraceRange = 1.f / traceRange;
    
    float prevTraceZ = positionVS.z;
    float prevSampleZ = prevTraceZ;
    
    // Debug
    uint sampleSpread = ceil(traceRange / MAX_TRACE_ITERATIONS);
    
    for (float i = 1.f; i <= traceRange; i += stride)
    {
        i = min(traceRange, i); // clamp to farthest pt
        float2 traceSS = positionSS.xy + dSS * i;
        
        float2 unswappedSS = (swapXY) ? traceSS.yx : traceSS.xy;
        float traceZ = GetPerspectiveCorrectDepth_Optimized(invStartPtDepth, invEndPtDepth, i * invTraceRange);
        
        // Read depth and convert it to linear depth
        float depth = HiZBuffer.Load(int3(unswappedSS.xy, 0)).x;
        if (depth == 0.f) // early return if sample out of viewport
            break;
        depth = HZB_LINEAR(depth, _ZBufferParams);
        depth += jitter; // to cut down on banding
        
#if defined(DEBUG_VIEW_SHOW_TRACE)
        if (isDebugTrace && (uint)i == _DebugViewStep * sampleSpread)
        {
            DebugData[0] = unswappedSS.x;
            DebugData[1] = unswappedSS.y;
            DebugData[2] = 0u;
        }
#endif
        
        // Test depth intersection
        float minTraceZ = min(traceZ, prevTraceZ);
        float maxTraceZ = max(traceZ, prevTraceZ);
        if (maxTraceZ > depth && minTraceZ < depth + thickness)
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
        prevSampleZ = depth;
    }
    return 0.f;
}

//
// Use combination of course search + binary refinement to cut down on depth samples
// This method is fast, but often misses intersections at edges of objects
//
float4 GetReflectColor_BinarySearch(uint3 tId, float3 positionVS, float4 positionSS, float3 reflectDirVS, float dither)
{
    //
    // Setup search
    //
    const bool isDebugTrace = (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y);
    
    const float maxRayLength = 25.f;
    const float stride = 16.f;
    const uint strideRefinementSteps = 3u; // EQ: log2 (stride) - 1
    
    float jitter = dither * 0.0111f;
    
    // Clip to near plane to avoid possible division by 0 during perspective divide
    float rayLength = ((positionVS.z + reflectDirVS.z * maxRayLength) < _ProjectionParams.y) ? (_ProjectionParams.y - positionVS.z) / reflectDirVS.z : maxRayLength;
    
    positionSS = PositionVSToSS(positionVS);
    float3 endPosVS = positionVS + reflectDirVS * rayLength;
    float4 endPosSS = PositionVSToSS(endPosVS);
    
    DDAParams params = GetDDAParams(positionSS, endPosSS);
    
    float4 course = CourseDDASearch(params, reflectDirVS, stride, jitter, isDebugTrace);
    //return float4(CameraColorIn[course.xy].rgb, course.w * 0.5f);
    
    //
    // Binary search (refinement)
    //
    
    // Setup sample pts for search
    float q0 = course.z - stride;
    float q1 = course.z;
    
    // Assume s1 is inside geometry
    DDAPt s0 = SampleDDAPt(params, q0, jitter);
    DDAPt s1 = SampleDDAPt(params, q1, jitter);
    int iter = stride;
    for (uint i = 0u; i < strideRefinementSteps; ++i)
    {
        float qMid = (q0 + q1) * 0.5f;
        DDAPt sMid = SampleDDAPt(params, qMid, jitter);
        if (sMid.traceDepth > sMid.sampleDepth)
        {
            s1 = sMid;
            q1 = qMid;
        }
        else
        {
            s0 = sMid;
            q0 = qMid;
        }
        iter--;
    }
    
#if !defined(SSR_ALLOWS_CAMERA_FACING_RAYS)
    // Do an extra check here. The camera-facing check helps, but due to the thickness param, check again.
    course.w *= (positionVS.z <= s1.sampleDepth);
#endif
    
    // Note - rather than calculating a new midpoint, use the last intersecting pt
    return float4(CameraColorIn[s1.traceSS.xy].rgb, course.w * 0.5f); // update confidence?
}

float4 GetReflectColor_HZB(uint3 tId, float3 positionVS, float4 positionSS, float3 reflectDirVS, float dither)
{
    const float maxRayLength = 5.f;
    
    // Clip to near plane to avoid possible division by 0 during perspective divide
    float rayLength = ((positionVS.z + reflectDirVS.z * maxRayLength) < _ProjectionParams.y) ? (_ProjectionParams.y - positionVS.z) / reflectDirVS.z : maxRayLength;
    
    positionSS += 0.5f; // move to center of pixel
    const float3 reflectDistSS = GetReflectionDistanceSS(positionVS, reflectDirVS, positionSS.xy, rayLength);
    
    const float3 invReflectDistSS = 1.f / reflectDistSS;
    const float2 halfSignDir = sign(reflectDistSS.xy) * 0.5f;
    int mip = 1;
    
    float pixelDist = min(abs(invReflectDistSS.x), abs(invReflectDistSS.y)); // travel distance to cover 1 pixel
    float traceDistance = pixelDist * 2.01f; // start with 1-pixel offset
    
    // Precalculate these (optimization)
    float invStartPtDepth = 1.f / positionVS.z;
    float invEndPtDepth = 1.f / (positionVS.z + reflectDirVS.z * rayLength);
    
    float testDist = saturate(dither);
    testDist = 0.5f;
    float trace0 = GetPerspectiveCorrectDepth_Optimized(invStartPtDepth, invEndPtDepth, testDist);
    float traceSolution0 = GetPerspectiveCorrectDepthLerp(invStartPtDepth, invEndPtDepth, trace0);
    float error = abs(testDist - traceSolution0);
    float isDepthCorrect = abs(error) < 0.001f ? 1.f : 0.f;
    //return error;
    //return float4(isDepthCorrect.xxx, 1.f);
    
    float2 tileIntersectOffset = 0.f;
    for (uint iter = 0u; iter < MAX_TRACE_ITERATIONS; ++iter)
    {
        float initTraceDistance = traceDistance;
        
        float2 traceSS = positionSS.xy + reflectDistSS.xy * saturate(traceDistance);
        uint2 ptId = (uint2)floor(traceSS.xy + tileIntersectOffset * 0);
        uint2 tileId = ptId >> (uint)mip;
        uint tilePixelSize = 1u << (uint)mip;
        
        //DEBUG_TRACE_PIXEL(tId, ptId, mip, iter);
#if defined(DEBUG_VIEW_SHOW_TRACE)
        if (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y && iter == _DebugViewStep) //_PixelSelection.z)
        {
            DebugData[0] = ptId.x;
            DebugData[1] = ptId.y;
            DebugData[2] = mip;
        }
#endif
        
        // Setup tile planes
        float2 planesUV = ((tileId + 0.5f) * tilePixelSize + (tilePixelSize + 1.f) * halfSignDir);
        
        // Calculate nearest intersection
        float2 intersectionSolutions = (planesUV - traceSS.xy) * invReflectDistSS.xy;
        float intersectDist = min(intersectionSolutions.x, intersectionSolutions.y);
        
        // 1/2 pixel offset for next sample
        if (intersectionSolutions.x < intersectionSolutions.y)
        {
            tileIntersectOffset.x = halfSignDir.x;
            //intersectDist += invReflectDistSS.x;
        }
        else
        {
            tileIntersectOffset.y = halfSignDir.y;
            //intersectDist += invReflectDistSS.y;
        }
        //tileIntersectOffset += (intersectionSolutions.x < intersectionSolutions.y) ? float2(halfSignDir.x, 0.f) : float2(0.f, halfSignDir.y);
        traceDistance = min(traceDistance + intersectDist, 1.f);
        
        // Calculate new trace Z
        float traceZ = GetPerspectiveCorrectDepth_Optimized(invStartPtDepth, invEndPtDepth, traceDistance);
            
        // Calculate depth plane intersection (for starting tile)
        float depth = HiZBuffer.Load(int3(tileId.xy, mip)).x;
        if (depth == 0.f) // early return if sample out of viewport
            break;
        depth = HZB_LINEAR(depth, _ZBufferParams);
        
        [branch]
        if (traceZ < depth)
        {
            // If no intersection, increase mip
            mip = min(mip + 1, MAX_MIP);
        }
        else
        {
            // If intersection, go back in time and decrease mip
            
            // todo: this should NEVER be greater than traceDistance!
            float depthDist = GetPerspectiveCorrectDepthLerp(invStartPtDepth, invEndPtDepth, depth);
            
            if (depthDist < initTraceDistance)
            {
                //return float4(1.f, 0.5f, 0.f, 1.f);
            }
            if (depthDist > traceDistance + 0.001f)
            {
                return float4(1.f, 0.f, 0.f, 1.f);
            }
            if (depthDist < traceDistance + 0.001f)
            {
                //return float4(1.f, 0.f, 0.f, 1.f);
                traceDistance = max(initTraceDistance, depthDist);
            }
            
            //traceDistance = max(initTraceDistance, depthDist);
            //mip = max(mip - 1, 0);
            mip--;
            //tileIntersectOffset = 0.f;
            
            [branch]
            if (mip == -1)
            {
                // Resolve final hit
                traceSS = positionSS.xy + reflectDistSS.xy * saturate(traceDistance);
                ptId = (uint2) floor(traceSS.xy);
                
#if defined(DEBUG_VIEW_SHOW_TRACE)
                // Resolve final hit
                if (tId.x == _PixelSelection.x && tId.y == _PixelSelection.y)
                {
                    DebugData[3] = ptId.x;
                    DebugData[4] = ptId.y;
                }
#endif
                
                float4 reflectColor;
                reflectColor.rgb = CameraColorIn[ptId].rgb;
                reflectColor.a = 1;
                return reflectColor;
            }
        }
        
        ++iter;
    }
    
    if (iter >= MAX_TRACE_ITERATIONS)
    {
        //return float4(1.f, 0.f, 0.f, 1.f);
    }
    
    return 0.f;
}

[numthreads(16, 16, 1)]
void CSMain(uint3 tId : SV_DispatchThreadID)
{
    //if (tId.x >= (uint) _ScreenParams.x || tId.y >= (uint) _ScreenParams.y)
    //    return;
    
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
    
    const float dither = DitherTex.Load(int3(tId.xy % 8, 0));
    
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
    const float reflectivity = gbufferTex.w;// * pow(saturate(1.0 - dot(viewDirVS, -normalVS)), 3.0);
    
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
    
    bool allowSSR = true;
#if !defined(SSR_ALLOWS_CAMERA_FACING_RAYS)
    allowSSR = (reflectDirVS.z > 0.f);
#endif
    
    if (allowSSR)
    {
#if defined(SSR_METHOD_3D_RAYMARCH)
        reflectColor = GetReflectColor_3DRaymarch(tId, positionVS, reflectDirVS);
#elif defined(SSR_METHOD_NC_DDA)
        reflectColor = GetReflectColor_NC_DDA(tId, positionVS, positionSS, reflectDirVS, dither);
#elif defined(SSR_METHOD_BINARY_SEARCH)
        reflectColor = GetReflectColor_BinarySearch(tId, positionVS, positionSS, reflectDirVS, dither);
#elif defined(SSR_METHOD_HI_Z)
        reflectColor = GetReflectColor_HZB(tId, positionVS, positionSS, reflectDirVS, dither);
#endif
    }
    
    // todo: use fallback when reflectColor.a = 0
    
    float4 colorIn = CameraColorIn[tId.xy];
    CameraColorOut[tId.xy] = float4(colorIn.rgb + reflectColor.rgb * (reflectColor.a * reflectivity), colorIn.a);
    /*if (!allowSSR)
    {
        CameraColorOut[tId.xy] = float4(1, 0, 0, 0);
    }*/
    
    //CameraColorOut[tId.xy] = (_PixelSelection.z == 2u);
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
    else if (tId.x == DebugData[3] && tId.y == DebugData[4])
    {
        debugColor = float3(0, 1, 0);
    }
    else if (tId.x == DebugData[3] || tId.y == DebugData[4])
    {
        debugColor = lerp(debugColor.rgb, float3(1, 1, 1), 0.8f);
    }
    else if (DebugData[2] > 0u && any(mipMod == 0u))
    {
        // Tiles
        debugColor = lerp(debugColor, float3(0, 1, 0), 0.15f);

    }
    
    DebugOut[tId.xy] = float4(debugColor.rgb, colorIn.a);
#endif
    
}