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
		static dx::XMVECTOR GetGerstnerWaveOffset(const dx::XMVECTOR& positionWS, const float timeElapsed)
		{
			const float x = dx::XMVectorGetX(positionWS);
			const float z = dx::XMVectorGetZ(positionWS);

			// Fade out waves at horizon (looks better!)
			const float isHorizon = std::clamp((x * x + z * z) * 0.0003f, 0.0f, 1.0f);

			const float wM = 0.44f;
			const float qM = 0.85f;
			const float ampM = 0.25f * (1.0f - isHorizon);

			// todo: sync these settings with GPU

			dx::XMVECTOR offsetWS;
			if (BuoyancyDebugTest)
			{
				offsetWS = GerstnerWave(x, z, 0.8384436f, 0.5449883f, 1.0f * wM, 0.25f * qM, timeElapsed * 2.0f, ampM * 5.0f);
			}
			else
			{
				// Medium waves
				offsetWS = GerstnerWave(x, z, 0.8384436f, 0.5449883f, 1.0f * wM, 1.0f * qM, timeElapsed * 2.0f, ampM * 0.5f);
				// Quick waves
				offsetWS = dx::XMVectorAdd(offsetWS, GerstnerWave(x, z, 0.2576626f, -0.9662349f, 3.18923f * wM, 1.1389f * qM, timeElapsed * 3.91f, ampM * 0.75f));
				// Quick waves
				offsetWS = dx::XMVectorAdd(offsetWS, GerstnerWave(x, z, 0.6940289f, 0.7199471f, 3.18923f * wM, 1.1389f * qM, timeElapsed * 4.07f, ampM * 0.85f));
				// Very quick waves
				offsetWS = dx::XMVectorAdd(offsetWS, GerstnerWave(x, z, -0.9941821f, 0.1077120f, 7.18923f * wM, 0.7789f * qM, timeElapsed * 5.1f, ampM * 0.551f));
			}

			return offsetWS;
		}

		// Estimates vertical displacement at a point
		static float GetGerstnerWaveVerticalDisplacementEstimate(const dx::XMVECTOR& positionWS, const float timeElapsed, const u32 iterations)
		{
			// Height sampling references:
			// https://www.youtube.com/watch?v=e0gKrx5JEn8
			// https://www.youtube.com/watch?v=kGEqaX4Y4bQ

			dx::XMVECTOR offset = dx::XMVectorZero();
			for (u32 i = 0u; i < iterations; ++i)
			{
				auto iPt = dx::XMVectorAdd(positionWS, offset);
				auto iOffset = GetGerstnerWaveOffset(iPt, timeElapsed);
				offset = dx::XMVectorMultiply(iOffset, dx::XMVectorSet(-1.f, 1.f, -1.f, 1.f)); // save vertical offset in Y component
			}

			return dx::XMVectorGetY(offset);
		}

	private:
		static dx::XMVECTOR GerstnerWave(const float vPosX, const float vPosZ, const float Dx, const float Dz, const float W, const float Q, const float T, const float amp)
		{
			// cos(WiDi * (x, y) + Qt)
			const float theta = W * (Dx * vPosX + Dz * vPosZ) + T;
			const float s = std::sin(theta);
			const float c = std::cos(theta);

			return dx::XMVectorSet(Q * Dx * c * amp, s * amp, Q * Dz * c * amp, 1.0f);
		}
	};
}