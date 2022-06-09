#include "pch.h"
#include "RandomGenerator.h"

namespace gfx
{

	RandomGenerator::RandomGenerator()
		: m_pRandomDevice(std::make_unique<std::random_device>())
	{
		std::random_device& rd = *(m_pRandomDevice.get());
		m_pGenerator = std::make_unique<std::mt19937>(rd());

		m_pUniformFloatDistribution01 = std::make_unique<std::uniform_real_distribution<float>>(0.f, 1.f);
	}

	const int RandomGenerator::GetUniformInt(const int minInclusive, const int maxExclusive) const
	{
		const std::uniform_int_distribution<int> distribution(minInclusive, maxExclusive);
		return distribution(*m_pGenerator);
	}

	const float RandomGenerator::GetUniformFloat(const float minInclusive, const float maxExclusive) const
	{
		const std::uniform_real_distribution<float> distribution(minInclusive, maxExclusive);
		return distribution(*m_pGenerator);
	}

	const float RandomGenerator::GetUniformFloat01() const
	{
		return (*m_pUniformFloatDistribution01.get())(*m_pGenerator);
	}

	const dx::XMVECTOR RandomGenerator::GetRandomUnitVector(const float wValue)
	{
		const auto& f01 = (*m_pUniformFloatDistribution01.get());
		return dx::XMVector3Normalize(dx::XMVectorSet(
			f01(*m_pGenerator) * 2.f - 1.f,
			f01(*m_pGenerator) * 2.f - 1.f,
			f01(*m_pGenerator) * 2.f - 1.f,
			wValue
		));
	}

}