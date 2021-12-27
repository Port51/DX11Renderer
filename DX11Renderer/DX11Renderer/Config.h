#pragma once

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
	static inline ShadowType ShadowType = ShadowType::HardwarePCF;
	static inline int ShadowResolution = 256;
};