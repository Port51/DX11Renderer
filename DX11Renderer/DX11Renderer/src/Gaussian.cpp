#include "pch.h"
#include "Gaussian.h"

namespace gfx
{

	void Gaussian::GetGaussianWeights1D(std::vector<float>& weights, const float sigma)
	{
		UINT width = (weights.size() - 1u) / 2u;

		// Set center width first
		weights[width] = GetGaussianWeight1D(0.f, sigma);

		for (UINT i = 1u; i <= width; ++i)
		{
			float weight = GetGaussianWeight1D((float)i, sigma);
			weights[width + i] = weight;
			weights[width - i] = weight;
		}
	}

	float Gaussian::GetGaussianWeight1D(const float x, const float sigma)
	{
		const float sqrt2Pi = 2.5066282746f;
		return std::exp(-x * x / (2.f * sigma * sigma)) / (sqrt2Pi * sigma);
	}

}