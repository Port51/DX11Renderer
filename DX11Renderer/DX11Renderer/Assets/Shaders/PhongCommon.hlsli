#ifndef _PHONG_COMMON_INCLUDED
#define _PHONG_COMMON_INCLUDED

struct v2f
{
    float4 pos : SV_Position;
    float3 normalVS : Normal;
    float3 positionVS : TEXCOORD0;
    float3 positionWS : TEXCOORD1;
    float3 normalWS : TEXCOORD2;
    float3 tangentVS : TEXCOORD3;
    float2 uv0 : TEXCOORD4;
    float4 vertColor : TEXCOORD5;
    float4 positionNDC : TEXCOORD6;
    float4 rng : TEXCOORD7;
};

#endif