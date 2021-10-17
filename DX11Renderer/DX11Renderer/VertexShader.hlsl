// Notes:
// - Remember to change properties of this file so outputs to $(ProjectDir)%(Filename).cso
// - And to set shader type
// - Can also change shader model targeted

struct VSOut
{
	float4 color : Color;
	float4 pos : SV_POSITION;
};

cbuffer CBuf
{
	//row_major matrix transform; // another option - slightly slower on GPU
	float4x4 transform; // could be called "matrix"
};

VSOut main(float2 positionOS : Position, float4 color : Color)
{
	VSOut vso;
	// Originally right multiply, so vector on left
	// CPU stored in row-major, GPU column-major
	// Instead of transposing, move the transform to start of mul here
	vso.pos = mul(transform, float4(positionOS.x, positionOS.y, 0.0f, 1.0f));
	vso.color = color;
	return vso;
}