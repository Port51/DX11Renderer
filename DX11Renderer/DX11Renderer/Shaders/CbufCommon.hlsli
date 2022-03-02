#ifndef CBUF_COMMON_INCLUDED
#define CBUF_COMMON_INCLUDED

cbuffer PerFrameCB : register(b0)
{
    uint4 _PixelSelection; // (x, y, -, -)
    float4 _Time; // (t/20, t, t*2, t*3)
    float4 _SinTime; // sin(t/8), sin(t/4), sin(t/2), sin(t)
    float4 _CosTime; // cos(t/8), cos(t/4), cos(t/2), cos(t)
};

cbuffer GlobalTransformCB : register(b1)
{
    matrix _ViewMatrix;
    matrix _ProjMatrix;
    matrix _ViewProjMatrix;
    matrix _InvViewMatrix;
    matrix _InvProjMatrix;
    matrix _InvViewProjMatrix;
};

cbuffer PerCameraCB : register(b2)
{
    // x = 1 or -1 (-1 if projection is flipped)
    // y = near plane
    // z = far plane
    // w = 1/far plane
    float4 _ProjectionParams;
    
    // x = width
    // y = height
    // z = 1.0/width
    // w = 1.0/height
    float4 _ScreenParams;

    // Values used to linearize the Z buffer (http://www.humus.name/temp/Linearize%20depth.txt)
    // x = 1-far/near
    // y = far/near
    // z = x/far
    // w = y/far
    // or in case of a reversed depth buffer (UNITY_REVERSED_Z is 1)
    // x = -1+far/near
    // y = 1
    // z = x/far
    // w = 1/far
    float4 _ZBufferParams;

    // x = orthographic camera's width
    // y = orthographic camera's height
    // z = unused
    // w = 1.0 if camera is ortho, 0.0 if perspective
    float4 _OrthoParams;
    
    float4 _FrustumCornerDataVS;
    
    float4 _CameraPositionWS;
    
    float4 _ClusterPrecalc; // mult slice, add slice
    float4 _ClusterXYRemap;
};

cbuffer LightInputCB : register(b3)
{
    uint _VisibleLightCount;
    float4 _ShadowAtlasTexelResolution;
    float4 _ShadowCascadeSphere0;
    float4 _ShadowCascadeSphere1;
    float4 _ShadowCascadeSphere2;
    float4 _ShadowCascadeSphere3;
};

#endif