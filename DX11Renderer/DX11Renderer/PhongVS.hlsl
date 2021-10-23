cbuffer CBuf
{
	matrix model;
	matrix modelView;
	matrix modelViewProj;
};

struct VSOut
{
	float3 positionVS : Position;
	float3 normal : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal)
{
	VSOut vso;
	vso.positionVS = (float3) mul(modelView, float4(pos, 1.0f));
	vso.normal = mul((float3x3) modelView, n);
	vso.pos = mul(modelViewProj, float4(pos, 1.0f));
	return vso;
}