#include "pch.h"
#include "Bokeh.h"

namespace gfx
{
	void Bokeh::GetDiskPackedWeights(std::vector<float>& weights, const float a0, const float b0, const float a1, const float b1)
	{
		const UINT componentWidth = weights.size() / 4u;
		const UINT width = (componentWidth - 1u) / 2u;

		const std::vector<float> params{ a0, b0, a1, b1 };

		for (UINT i = 0u; i < 4u; ++i)
		{
			const UINT offset = componentWidth * i;
			const float a = i < 2u ? a0 : a1;
			const float b = i < 2u ? b0 : b1;
			const bool isReal = i % 2u == 0u;

			// Set center width first
			weights[offset + width] = isReal ? GetDiskRealWeight1D(0.f, a, b) : GetDiskImaginaryWeight1D(0.f, a, b);

			for (UINT x = 1u; x <= width; ++x)
			{
				float weight = isReal ? GetDiskRealWeight1D((float)x, a, b) : GetDiskImaginaryWeight1D((float)x, a, b);
				weights[offset + width + x] = weight;
				weights[offset + width - x] = weight;
			}
		}
		
	}

	void Bokeh::GetDiskRealWeights1D(std::vector<float>& weights, const float a, const float b)
	{
		UINT width = (weights.size() - 1u) / 2u;

		// Set center width first
		weights[width] = GetDiskRealWeight1D(0.f, a, b);

		for (UINT i = 1u; i <= width; ++i)
		{
			float weight = GetDiskRealWeight1D((float)i, a, b);
			weights[width + i] = weight;
			weights[width - i] = weight;
		}
	}

	float Bokeh::GetDiskRealWeight1D(const float x, const float a, const float b)
	{
		// Eq: F(x) = e^(-ax^2) (cos(bx^2) + i * sin(bx^2))
		return std::exp(-a * x * x) * std::cos(b * x * x);
	}

	void Bokeh::GetDiskImaginaryWeights1D(std::vector<float>& weights, const float a, const float b)
	{
		UINT width = (weights.size() - 1u) / 2u;

		// Set center width first
		weights[width] = GetDiskImaginaryWeight1D(0.f, a, b);

		for (UINT i = 1u; i <= width; ++i)
		{
			float weight = GetDiskImaginaryWeight1D((float)i, a, b);
			weights[width + i] = weight;
			weights[width - i] = weight;
		}
	}

	float Bokeh::GetDiskImaginaryWeight1D(const float x, const float a, const float b)
	{
		// Eq: F(x) = e^(-ax^2) (cos(bx^2) + i * sin(bx^2))
		return std::exp(-a * x * x) * std::sin(b * x * x);
	}

}