#include "pch.h"
#include "Bokeh.h"

namespace gfx
{
	void Bokeh::GetDiskPackedWeights(std::vector<float>& weights, const float a0, const float b0, const float a1, const float b1, const float a2, const float b2)
	{
		const UINT componentWidth = weights.size() / 6u;
		const UINT width = (componentWidth - 1u) / 2u;

		const std::vector<float> params{ a0, b0, a1, b1, a2, b2 };

		for (UINT i = 0u; i < 6u; ++i)
		{
			const UINT offset = componentWidth * i;
			const UINT paramIdx = (i >> 1u) << 1u;
			const float a = params[paramIdx + 0u];
			const float b = params[paramIdx + 1u];
			const bool isReal = (i % 2u == 0u);

			// Set center width first
			weights[offset + width] = isReal ? GetDiskRealWeight1D(0.f, a, b) : GetDiskImaginaryWeight1D(0.f, a, b);

			for (UINT x = 1u; x <= width; ++x)
			{
				const float scaledX = x / 13.f;
				const float weight = isReal ? GetDiskRealWeight1D((float)scaledX, a, b) : GetDiskImaginaryWeight1D((float)scaledX, a, b);
				weights[offset + width + x] = weight;
				weights[offset + width - x] = weight;
			}
		}
		
	}

	void Bokeh::GetDiskRealWeights1D(std::vector<float>& weights, UINT componentIdx, UINT elements, const float a, const float b, const float xScale, const bool normalize)
	{
		const UINT width = (elements - 1u) / 2u;
		const UINT startIdx = componentIdx * elements;
		const UINT midIdx = startIdx + width;

		// Set center width first
		weights[midIdx] = GetDiskRealWeight1D(0.f, a, b);
		float sum = weights[midIdx];

		for (UINT x = 1u; x <= width; ++x)
		{
			float weight = GetDiskRealWeight1D((float)x * xScale, a, b);
			weights[midIdx + x] = weight;
			weights[midIdx - x] = weight;
			sum += weight * 2.f;
		}

		if (normalize)
		{
			const float normalizeFactor = 1.f / sum;
			for (UINT i = startIdx; i < startIdx + elements; ++i)
			{
				weights[i] *= normalizeFactor;
			}
		}
	}

	float Bokeh::GetDiskRealWeight1D(const float x, const float a, const float b)
	{
		// Eq: F(x) = e^(-ax^2) (cos(bx^2) + i * sin(bx^2))
		return std::exp(-a * x * x) * std::cos(b * x * x);
	}

	void Bokeh::GetDiskImaginaryWeights1D(std::vector<float>& weights, UINT componentIdx, UINT elements, const float a, const float b, const float xScale, const bool normalize)
	{
		const UINT width = (elements - 1u) / 2u;
		const UINT startIdx = componentIdx * elements;
		const UINT midIdx = startIdx + width;

		// Set center width first
		weights[midIdx] = GetDiskImaginaryWeight1D(0.f, a, b);
		float sum = weights[midIdx];

		for (UINT x = 1u; x <= width; ++x)
		{
			float weight = GetDiskImaginaryWeight1D((float)x * xScale, a, b);
			weights[midIdx + x] = weight;
			weights[midIdx - x] = weight;
			sum += weight * 2.f;
		}

		if (normalize)
		{
			const float normalizeFactor = 1.f / sum;
			for (UINT i = startIdx; i < startIdx + elements; ++i)
			{
				weights[i] *= normalizeFactor;
			}
		}
	}

	float Bokeh::GetDiskImaginaryWeight1D(const float x, const float a, const float b)
	{
		// Eq: F(x) = e^(-ax^2) (cos(bx^2) + i * sin(bx^2))
		return std::exp(-a * x * x) * std::sin(b * x * x);
	}

}