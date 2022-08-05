#pragma once
#include "CommonHeader.h"

namespace gfx
{
	// CPU estimation of Gerstner waves
	class GerstnerWaves
	{
	private:
		// This is used for demonstrating buoyancy with large, simple waves
		static const bool BuoyancyDebugTest = false;
	public:
		static dx::XMVECTOR GetGerstnerWaveOffset(const dx::XMVECTOR& positionWS, const float timeElapsed);

		// Estimates vertical displacement at a point
		static float GetGerstnerWaveVerticalDisplacementEstimate(const dx::XMVECTOR& positionWS, const float timeElapsed, const u32 iterations);

	private:
		static dx::XMVECTOR GerstnerWave(const float vPosX, const float vPosZ, const float Dx, const float Dz, const float W, const float Q, const float T, const float amp);
	};
}