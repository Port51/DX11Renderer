#include "pch.h"
#include "RenderConstants.h"
#include <functional>

namespace gfx
{
	const float RenderConstants::PI = 3.141592654f;
	const float RenderConstants::PI_2 = 6.283185308f;
	const float RenderConstants::INV_PI = 0.31830988614f;
	const float RenderConstants::INV_PI_2 = 0.15915494307f;
	const float RenderConstants::DEG_TO_RAD = 0.01745329252f;
	const float RenderConstants::RAD_TO_DEG = 57.2957795056f;
	const std::vector<ID3D11Buffer*> RenderConstants::NullBufferArray(256, nullptr);
	const std::vector<ID3D11ShaderResourceView*> RenderConstants::NullSRVArray(256, nullptr);
	const std::vector<ID3D11UnorderedAccessView*> RenderConstants::NullUAVArray(256, nullptr);
	const std::vector<ID3D11SamplerState*> RenderConstants::NullSamplerArray(256, nullptr);

	std::unordered_map<RenderPassType, std::string> RenderPassConstants::m_renderPassNameByEnum
	{
		{ RenderPassType::PerCameraRenderPass,			"PerCameraRenderPass" },
		{ RenderPassType::ShadowRenderPass,				"ShadowRenderPass" },
		{ RenderPassType::DepthPrepassRenderPass,		"DepthPrepassRenderPass" },
		{ RenderPassType::HiZRenderPass,				"HiZRenderPass" },
		{ RenderPassType::GBufferRenderPass,			"GBufferRenderPass" },
		{ RenderPassType::SSAORenderPass,				"SSAORenderPass" },
		{ RenderPassType::TiledLightingRenderPass,		"TiledLightingRenderPass" },
		{ RenderPassType::ClusteredLightingRenderPass,	"ClusteredLightingRenderPass" },
		{ RenderPassType::OpaqueRenderPass,				"OpaqueRenderPass" },
		{ RenderPassType::SkyboxRenderPass,				"SkyboxRenderPass" },
		{ RenderPassType::TransparentRenderPass,		"TransparentRenderPass" },
		{ RenderPassType::CreateDownsampledX2Texture,	"CreateDownsampledX2Texture" },
		{ RenderPassType::DepthOfFieldRenderPass,		"DepthOfFieldRenderPass" },
		{ RenderPassType::BloomRenderPass,				"BloomRenderPass" },
		{ RenderPassType::SSRRenderPass,				"SSRRenderPass" },
		{ RenderPassType::FXAARenderPass,				"FXAARenderPass" },
		{ RenderPassType::DitherRenderPass,				"DitherRenderPass" },
		{ RenderPassType::TonemappingRenderPass,		"TonemappingRenderPass" },
		{ RenderPassType::FinalBlitRenderPass,			"FinalBlitRenderPass" },
	};

	std::unordered_map<RenderPassType, std::size_t> RenderPassConstants::m_renderPassHashByEnum
	{
		{ RenderPassType::PerCameraRenderPass,			std::hash<std::string>{}("PerCameraRenderPass") },
		{ RenderPassType::ShadowRenderPass,				std::hash<std::string>{}("ShadowRenderPass") },
		{ RenderPassType::DepthPrepassRenderPass,		std::hash<std::string>{}("DepthPrepassRenderPass") },
		{ RenderPassType::HiZRenderPass,				std::hash<std::string>{}("HiZRenderPass") },
		{ RenderPassType::GBufferRenderPass,			std::hash<std::string>{}("GBufferRenderPass") },
		{ RenderPassType::SSAORenderPass,				std::hash<std::string>{}("SSAORenderPass") },
		{ RenderPassType::TiledLightingRenderPass,		std::hash<std::string>{}("TiledLightingRenderPass") },
		{ RenderPassType::ClusteredLightingRenderPass,	std::hash<std::string>{}("ClusteredLightingRenderPass") },
		{ RenderPassType::OpaqueRenderPass,				std::hash<std::string>{}("OpaqueRenderPass") },
		{ RenderPassType::SkyboxRenderPass,				std::hash<std::string>{}("SkyboxRenderPass") },
		{ RenderPassType::TransparentRenderPass,		std::hash<std::string>{}("TransparentRenderPass") },
		{ RenderPassType::CreateDownsampledX2Texture,	std::hash<std::string>{}("CreateDownsampledX2Texture") },
		{ RenderPassType::DepthOfFieldRenderPass,		std::hash<std::string>{}("DepthOfFieldRenderPass") },
		{ RenderPassType::BloomRenderPass,				std::hash<std::string>{}("BloomRenderPass") },
		{ RenderPassType::SSRRenderPass,				std::hash<std::string>{}("SSRRenderPass") },
		{ RenderPassType::FXAARenderPass,				std::hash<std::string>{}("FXAARenderPass") },
		{ RenderPassType::DitherRenderPass,				std::hash<std::string>{}("DitherRenderPass") },
		{ RenderPassType::TonemappingRenderPass,		std::hash<std::string>{}("TonemappingRenderPass") },
		{ RenderPassType::FinalBlitRenderPass,			std::hash<std::string>{}("FinalBlitRenderPass") },
	};

	std::unordered_map<std::size_t, RenderPassType> RenderPassConstants::m_renderPassEnumByHash
	{
		{ std::hash<std::string>{}("PerCameraRenderPass"),			RenderPassType::PerCameraRenderPass },
		{ std::hash<std::string>{}("ShadowRenderPass"),				RenderPassType::ShadowRenderPass },
		{ std::hash<std::string>{}("DepthPrepassRenderPass"),		RenderPassType::DepthPrepassRenderPass },
		{ std::hash<std::string>{}("HiZRenderPass"),				RenderPassType::HiZRenderPass },
		{ std::hash<std::string>{}("GBufferRenderPass"),			RenderPassType::GBufferRenderPass },
		{ std::hash<std::string>{}("SSAORenderPass"),				RenderPassType::SSAORenderPass},
		{ std::hash<std::string>{}("TiledLightingRenderPass"),		RenderPassType::TiledLightingRenderPass },
		{ std::hash<std::string>{}("ClusteredLightingRenderPass"),	RenderPassType::ClusteredLightingRenderPass },
		{ std::hash<std::string>{}("OpaqueRenderPass"),				RenderPassType::OpaqueRenderPass },
		{ std::hash<std::string>{}("SkyboxRenderPass"),				RenderPassType::SkyboxRenderPass },
		{ std::hash<std::string>{}("TransparentRenderPass"),		RenderPassType::TransparentRenderPass },
		{ std::hash<std::string>{}("CreateDownsampledX2Texture"),	RenderPassType::CreateDownsampledX2Texture },
		{ std::hash<std::string>{}("DepthOfFieldRenderPass"),		RenderPassType::DepthOfFieldRenderPass },
		{ std::hash<std::string>{}("BloomRenderPass"),				RenderPassType::BloomRenderPass },
		{ std::hash<std::string>{}("SSRRenderPass"),				RenderPassType::SSRRenderPass },
		{ std::hash<std::string>{}("FXAARenderPass"),				RenderPassType::FXAARenderPass },
		{ std::hash<std::string>{}("DitherRenderPass"),				RenderPassType::DitherRenderPass },
		{ std::hash<std::string>{}("TonemappingRenderPass"),		RenderPassType::TonemappingRenderPass },
		{ std::hash<std::string>{}("FinalBlitRenderPass"),			RenderPassType::FinalBlitRenderPass },
	};

	std::string RenderPassConstants::GetRenderPassName(RenderPassType passType)
	{
		return m_renderPassNameByEnum.at(passType);
	}

	std::size_t RenderPassConstants::GetRenderPassHash(RenderPassType passType)
	{
		return m_renderPassHashByEnum.at(passType);
	}

	RenderPassType RenderPassConstants::GetRenderPassType(std::string passName)
	{
		std::size_t hash = std::hash<std::string>{}(std::move(passName));
		if (m_renderPassEnumByHash.find(hash) != m_renderPassEnumByHash.end())
		{
			return m_renderPassEnumByHash.at(hash);
		}
		return RenderPassType::Undefined;
	}

	bool RenderPassConstants::TryGetRenderPassName(RenderPassType passType, std::string& result)
	{
		if (m_renderPassNameByEnum.find(passType) != m_renderPassNameByEnum.end())
		{
			result = m_renderPassNameByEnum.at(passType);
			return true;
		}
		return false;
	}

	bool RenderPassConstants::TryGetRenderPassHash(RenderPassType passType, std::size_t& result)
	{
		if (m_renderPassHashByEnum.find(passType) != m_renderPassHashByEnum.end())
		{
			result = m_renderPassHashByEnum.at(passType);
			return true;
		}
		return false;
	}

	bool RenderPassConstants::TryGetRenderPassType(std::string passName, RenderPassType& result)
	{
		std::size_t hash = std::hash<std::string>{}(std::move(passName));
		if (m_renderPassEnumByHash.find(hash) != m_renderPassEnumByHash.end())
		{
			result = m_renderPassEnumByHash.at(hash);
			return true;
		}
		return false;
	}

}