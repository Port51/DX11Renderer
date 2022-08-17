#pragma once

namespace gfx
{
	class Config
	{
	public:
		enum ShadowType
		{
			Hard,
			PCF,
			Moment,
		};
		enum AAType
		{
			None,
			FXAA,
		};

	public:
		static constexpr bool LinearLighting = true;
		static constexpr bool UseWireframe = false;

		static constexpr ShadowType ShadowType = ShadowType::PCF;
		static constexpr AAType AAType = AAType::FXAA;

		static constexpr int MsaaSamples = 1;
		static constexpr int MsaaQuality = 0;

		// Each tile represents a shadowmap, which can be a spotlight shadow, cubemap face, or a specific directional cascade
		static constexpr int ShadowAtlasResolution = 4096;
		// Size of each tile
		static constexpr int ShadowAtlasTileResolution = ShadowAtlasResolution / 4;
		// Number of tiles on a side
		static constexpr int ShadowAtlasTileDimension = ShadowAtlasResolution / ShadowAtlasTileResolution;
		// Total number of tiles
		static constexpr int ShadowAtlasTileCount = ShadowAtlasResolution * ShadowAtlasResolution / (ShadowAtlasTileResolution * ShadowAtlasTileResolution);

		// Cascades per dir light
		static constexpr int ShadowCascades = 4;
		static std::vector<float> ShadowCascadeDistances;
		// Distance to start cascades in reverse direction of light (increase to receive more shadows from behind the camera)
		static constexpr float ShadowCascadeOffset = 20.f;

	};
}