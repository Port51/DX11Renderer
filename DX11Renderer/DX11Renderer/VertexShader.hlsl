// Notes:
// - Remember to change properties of this file so outputs to $(ProjectDir)%(Filename).cso
// - And to set shader type
// - Can also change shader model targeted

struct VSOut
{
	float4 color : Color;
	float4 pos : SV_POSITION;
};

VSOut main(float2 positionOS : Position, float4 color : Color)
{
	VSOut vso;
	vso.pos = float4(positionOS.x, positionOS.y, 0.0f, 1.0f);
	vso.color = color;
	return vso;
}