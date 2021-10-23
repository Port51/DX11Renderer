cbuffer CBuf
{
	matrix model;
	matrix modelViewProj;
};

struct VSOut
{
	float3 worldPos : Position;
	float3 normal : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal)
{
	VSOut vso;
	vso.worldPos = (float3) mul(model, float4(pos, 1.0f));
	vso.normal = mul((float3x3) model, n);
	vso.pos = mul(modelViewProj, float4(pos, 1.0f));
	return vso;
}