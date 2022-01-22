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

		// Each tile represents a shadowmap, which can be a spotlight shadow, cubemap face, or a specific directional cascade
		static inline int ShadowAtlasResolution = 512;
		// Size of each tile
		static inline int ShadowAtlasTileResolution = 256;
		// Number of tiles on a side
		static inline int ShadowAtlasTileDimension = ShadowAtlasResolution / ShadowAtlasTileResolution;
		// Total number of tiles
		static inline int ShadowAtlasTileCount = ShadowAtlasResolution * ShadowAtlasResolution / (ShadowAtlasTileResolution * ShadowAtlasTileResolution);
	};
}