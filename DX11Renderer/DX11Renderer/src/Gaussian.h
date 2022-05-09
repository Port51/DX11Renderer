#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Gaussian
	{
	public:
		static void GetGaussianWeights1D(std::vector<float>& weights, const float sigma);
	private:
		static float GetGaussianWeight1D(const float x, const float sigma);
	};
}