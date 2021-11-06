#ifndef _PHONG_COMMON_INCLUDED
#define _PHONG_COMMON_INCLUDED

struct v2f
{
    float4 pos : SV_Position;
    float3 normalVS : Normal;
    float3 positionVS : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
    float3 tangentVS : TEXCOORD2;
    float2 uv0 : TEXCOORD3;
};

#endif