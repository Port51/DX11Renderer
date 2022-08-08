#include "pch.h"
#include "Bokeh.h"

namespace gfx
{

	void Bokeh::GetDiskRealWeights1D(std::vector<float>& weights, const UINT componentIdx, const UINT elements, const float a, const float b, const float xScale, const bool normalize)
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
			const float normalizeFactor = 1.f / std::sqrt(sum);
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

	void Bokeh::GetDiskImaginaryWeights1D(std::vector<float>& weights, const UINT componentIdx, const UINT elements, const float a, const float b, const float xScale, const bool normalize)
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

	const float Bokeh::GetDiskAccumulation(const std::vector<float>& weights, const UINT startComponentIdx, const UINT componentCount, const UINT elementsPerComponent, const float combineRealFactor, const float combineImaginaryFactor)
	{
		float accu = 1.f;
		for (UINT cIdx = startComponentIdx; cIdx < startComponentIdx + componentCount; ++cIdx)
		{
			// Calculate complex sum
			float sumR = 0.f;
			float sumI = 0.f;
			for (UINT x = 0u; x < elementsPerComponent; ++x)
			{
				for (UINT y = 0u; y < elementsPerComponent; ++y)
				{
					// Sum of complex numbers is done component-wise
					// P + Q = (Pr + Qr) + (Pi + Qi) * i
					const float xReal = weights[elementsPerComponent * 2u * cIdx + x];
					const float xImag = weights[elementsPerComponent * 2u * cIdx + elementsPerComponent + x];
					const float yReal = weights[elementsPerComponent * 2u * cIdx + y];
					const float yImag = weights[elementsPerComponent * 2u * cIdx + elementsPerComponent + y];
					sumR += (xReal * yReal - xImag * yImag);
					sumI += (xReal * yImag + xImag * yReal);
				}
			}
			accu = accu * (sumR * combineRealFactor + sumI * combineImaginaryFactor); // todo: add weights
		}
		return accu * 1.8f;
	}

	const void Bokeh::ApplyScaleToDisk(std::vector<float>& weights, const UINT startComponentIdx, const UINT componentCount, const UINT elementsPerComponent, const float scaleFactor)
	{
		for (UINT i = startComponentIdx * elementsPerComponent * 2u; i < (startComponentIdx + componentCount) * elementsPerComponent * 2u; ++i)
		{
			weights[i] *= scaleFactor;
		}
	}

	float Bokeh::GetDiskImaginaryWeight1D(const float x, const float a, const float b)
	{
		// Eq: F(x) = e^(-ax^2) (cos(bx^2) + i * sin(bx^2))
		return std::exp(-a * x * x) * std::sin(b * x * x);
	}

}