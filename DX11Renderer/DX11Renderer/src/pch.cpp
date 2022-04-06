#include "pch.h"

float RandomRange(float min, float max)
{
	return ((float)rand() / (RAND_MAX)) * (max - min) + min;
}

dx::XMFLOAT3 RandomColorRGB()
{
	return std::move(dx::XMFLOAT3(RandomRange(0.f, 1.f), RandomRange(0.f, 1.f), RandomRange(0.f, 1.f)));
}

dx::XMFLOAT4 RandomColorRGBA()
{
	auto rgb = RandomColorRGB();
	return std::move(dx::XMFLOAT4(rgb.x, rgb.y, rgb.z, RandomRange(0.f, 1.f)));
}

dx::XMFLOAT3 RandomSaturatedColorRGB()
{
	return std::move(dx::XMFLOAT3(RandomRange(0.f, 1.f), RandomRange(0.f, 1.f), RandomRange(0.f, 1.f)));
}

dx::XMFLOAT4 RandomSaturatedColorRGBA()
{
	auto rgb = RandomSaturatedColorRGB();
	return std::move(dx::XMFLOAT4(rgb.x, rgb.y, rgb.z, RandomRange(0.f, 1.f)));
}

dx::XMFLOAT3 DecomposeMatrixTranslation(const dx::XMFLOAT4X4& trsMatrix)
{
	return std::move(dx::XMFLOAT3(trsMatrix._14, trsMatrix._24, trsMatrix._34));
}
