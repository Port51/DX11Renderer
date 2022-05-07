
// todo: use for alpha-cut materials

struct v2f
{
    float4 pos : SV_POSITION;
    float depth : TEXCOORD0;
};

float4 main(v2f i) : SV_TARGET
{
    return 0;
}