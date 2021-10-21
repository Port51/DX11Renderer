// Notes:
// - Remember to change properties of this file so outputs to $(ProjectDir)%(Filename).cso
// - And to set shader type
// - Can also change shader model targeted

cbuffer CBuf
{
	float4 faceColors[6];
};

// SV_PrimitiveID is per-triangle
float4 main(uint tid : SV_PrimitiveID) : SV_TARGET
{
	return faceColors[(tid >> 1u) % 6u];
}