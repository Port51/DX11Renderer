#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Bokeh
	{
	public:
		// Pack x2 components into x1 vector in order: (C0 real, C0 imaginary, C1 real, C1 imaginary)
		static void GetDiskPackedWeights(std::vector<float>& weights, const float a0, const float b0, const float a1, const float b1);
		static void GetDiskRealWeights1D(std::vector<float>& weights, const float a, const float b);
		static void GetDiskImaginaryWeights1D(std::vector<float>& weights, const float a, const float b);
	private:
		static float GetDiskRealWeight1D(const float x, const float a, const float b);
		static float GetDiskImaginaryWeight1D(const float x, const float a, const float b);
	};
}