#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"
#include <random>

/*namespace std
{
	class random_device;
	class mt19937;

	template<typename Type>
	class uniform_real_distribution;
}*/

namespace gfx
{

	class RandomGenerator
	{
	public:
		RandomGenerator();
	public:
		const int GetUniformInt(const int minInclusive, const int maxExclusive);
		const float GetUniformFloat(const float minInclusive, const float maxExclusive);
		const float GetUniformFloat01();
		const dx::XMVECTOR GetRandomUnitVector(const float wValue);
	private:
		std::unique_ptr<std::random_device> m_pRandomDevice;
		std::unique_ptr<std::mt19937> m_pGenerator;
		std::unique_ptr<std::uniform_real_distribution<float>> m_pUniformFloatDistribution01;
	};
}