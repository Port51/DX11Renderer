#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Gaussian
	{
	public:
		// Calculate sigma such that far elements <= maxError
		static void GetGaussianWeights1D(std::vector<float>& weights, const float sigma);
		static float GetGaussianWeight1D(const float x, const float sigma);
	};
}