#pragma once

namespace gfx
{
	class Config
	{
	public:
		enum ShadowType
		{
			Hard,
			HardwarePCF,
			Moment
		};
	public:
		static inline ShadowType ShadowType = ShadowType::Hard;
		static inline int ShadowResolution = 256;
	};
}