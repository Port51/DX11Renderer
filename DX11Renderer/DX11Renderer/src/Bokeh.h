#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Bokeh
	{
	public:
		static void GetDiskRealWeights1D(std::vector<float>& weights, const UINT componentIdx, const UINT elements, const float a, const float b, const float xScale, const bool normalize);
		static void GetDiskImaginaryWeights1D(std::vector<float>& weights, const UINT componentIdx, const UINT elements, const float a, const float b, const float xScale, const bool normalize);
		static const float GetDiskAccumulation(const std::vector<float>& weights, const UINT startComponentIdx, const UINT componentCount, const UINT elementsPerComponent, const float combineRealFactor, const float combineImaginaryFactor);
		static const void ApplyScaleToDisk(std::vector<float>& weights, const UINT startComponentIdx, const UINT componentCount, const UINT elementsPerComponent, const float scaleFactor);
	private:
		static float GetDiskRealWeight1D(const float x, const float a, const float b);
		static float GetDiskImaginaryWeight1D(const float x, const float a, const float b);
	};
}