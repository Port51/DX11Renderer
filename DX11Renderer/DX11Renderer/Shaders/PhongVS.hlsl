
#include "PhongCommon.hlsli"

cbuffer CBuf
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
};

v2f main(float3 pos : Position, float3 n : Normal)
{
    v2f vso;
    vso.positionVS = (float3) mul(modelView, float4(pos, 1.0f));
    vso.normalWS = n;
    vso.normalVS = mul((float3x3) modelView, n);
	vso.pos = mul(modelViewProj, float4(pos, 1.0f));
	return vso;
}