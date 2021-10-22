Texture2D tex : register(t0); // slot 0
SamplerState tex_Sampler;// : register(t0);

float4 main(float2 tc : TexCoord) : SV_Target
{
	return tex.Sample(tex_Sampler,tc);
}