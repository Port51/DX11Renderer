#ifndef _LIGHT_STRUCTS_INCLUDED
#define _LIGHT_STRUCTS_INCLUDED

struct StructuredLight
{
    float4 positionVS_range;
    float4 color_intensity;
    float4 direction;
    float4 data0;
};

struct StructuredShadow
{
    float4 positionVS_range;
    float4 color_intensity;
    float4 direction;
    float4 data0;
};

#endif