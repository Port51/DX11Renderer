// Notes:
// - Remember to change properties of this file so outputs to $(ProjectDir)%(Filename).cso
// - And to set shader type
// - Can also change shader model targeted

float4 main(float2 positionOS : Position) : SV_POSITION
{
	return float4(positionOS.x, positionOS.y, 0.0f, 1.0f);
}