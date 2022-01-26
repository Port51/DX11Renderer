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
		static inline int ShadowAtlasResolution = 4096;
		// Size of each tile
		static inline int ShadowAtlasTileResolution = ShadowAtlasResolution / 4;
		// Number of tiles on a side
		static inline int ShadowAtlasTileDimension = ShadowAtlasResolution / ShadowAtlasTileResolution;
		// Total number of tiles
		static inline int ShadowAtlasTileCount = ShadowAtlasResolution * ShadowAtlasResolution / (ShadowAtlasTileResolution * ShadowAtlasTileResolution);

		// Cascades per dir light
		static inline int ShadowCascades = 4;
		static std::vector<float> ShadowCascadeDistances;
		// Distance to start cascades in reverse direction of light (increase to receive more shadows from behind the camera)
		static inline float ShadowCascadeOffset = 20.f;
	};
}