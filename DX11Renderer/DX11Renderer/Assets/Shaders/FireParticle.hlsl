#include "CbufCommon.hlsli"

struct attrib
{
    float3 instancePosition : POSITIONVS;
};

struct v2g
{
    float4 positionVS : TEXCOORD0;
};

struct g2f
{
    float4 positionVS : TEXCOORD0;
};

v2g vert(attrib i)
{
    v2g o;
    //o.positionHCS = mul(_ProjMatrix, float4(i.instancePosition, 1.0f));
    // todo: clustered lighting
    return o;
}

[maxvertexcount(4)]
void geom(point v2g input[1], inout TriangleStream<g2f> outStream)
{
    
    outStream.RestartStrip();
}

float4 frag(g2f i) : SV_Target
{
    return 1.f;
}