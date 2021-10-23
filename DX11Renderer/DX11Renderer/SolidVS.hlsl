cbuffer CBuf
{
	matrix model;
	matrix modelViewProj;
};

float4 main(float3 pos : Position) : SV_Position
{
	return mul(modelViewProj, float4(pos, 1.0f));
}