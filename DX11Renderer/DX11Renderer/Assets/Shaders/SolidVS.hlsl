cbuffer CBuf
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
};

struct attrib
{
    float3 pos : Position;
    float3 n : Normal;
    float3 t : Tangent;
    float2 uv0 : TEXCOORD0;
};

float4 main(attrib i) : SV_Position
{
	return mul(modelViewProj, float4(i.pos, 1.0f));
}