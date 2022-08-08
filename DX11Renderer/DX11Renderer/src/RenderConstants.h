#pragma once
#include "CommonHeader.h"
#include <unordered_map>
#include <string>

struct ID3D11BUFFER;

namespace gfx
{
	enum RenderPassType : int
	{
		Undefined,
		Subpass,
		PerCameraRenderPass,
		ShadowRenderPass,
		DepthPrepassRenderPass,
		HiZRenderPass,
		GBufferRenderPass,
		ParticleComputeRenderPass,
		TiledLightingRenderPass,
		ClusteredLightingRenderPass,
		OpaqueRenderPass,
		SkyboxRenderPass,
		TransparentRenderPass,
		CreateDownsampledX2Texture,
		DepthOfFieldRenderPass,
		BloomRenderPass,
		ParticleRenderPass,
		SSAORenderPass,
		SSRRenderPass,
		FXAARenderPass,
		DitherRenderPass,
		TonemappingRenderPass,
		FinalBlitRenderPass
	};

	class RenderBindingType
	{
	public:
		enum RenderBindingTypeEnum : u8
		{
			IA_InputLayout, IA_VertexBuffer, IA_IndexBuffer, IA_Topology,
			CS_SRV, CS_UAV, CS_CB, CS_Shader, CS_Sampler,
			VS_SRV, VS_UAV, VS_CB, VS_Shader, VS_Sampler,
			GS_SRV, GS_UAV, GS_CB, GS_Shader, GS_Sampler,
			RS_State,
			PS_SRV, PS_UAV, PS_CB, PS_Shader, PS_Sampler,
			OM_DepthStencilState,
		};
	};

	class RenderSlots
	{
	public:
		// Shared by all stages
		static const UINT Global_PointWrapSampler = 0u;
		static const UINT Global_PointClampSampler = 1u;
		static const UINT Global_PointMirrorSampler = 2u;
		static const UINT Global_BilinearClampSampler = 3u;
		static const UINT Global_BilinearWrapSampler = 4u;
		static const UINT Global_BilinearMirrorSampler = 5u;

		static const UINT CS_PerFrameCB = 0u;
		static const UINT CS_GlobalTransformsCB = 1u;
		static const UINT CS_PerCameraCB = 2u;
		static const UINT CS_LightInputCB = 3u;
		static const UINT CS_FreeCB = 4u; // 1st available general-purpose slot

		static const UINT CS_LightDataSRV = 0u;
		static const UINT CS_LightShadowDataSRV = 1u;
		static const UINT CS_GbufferNormalRoughSRV = 2u;
		static const UINT CS_FreeSRV = 3u; // 1st available general-purpose slot
		static const UINT CS_FreeSPL = 6u; // 1st available general-purpose slot
		static const UINT CS_FreeUAV = 0u; // 1st available general-purpose slot

		static const UINT VS_PerFrameCB = 0u;
		static const UINT VS_GlobalTransformsCB = 1u;
		static const UINT VS_PerCameraCB = 2u;
		static const UINT VS_PerObjectTransformCB = 3u;
		static const UINT VS_FreeCB = 4u; // 1st available general-purpose slot
		static const UINT VS_FreeSPL = 6u; // 1st available general-purpose slot
		static const UINT VS_FreeSRV = 0u; // 1st available general-purpose slot

		static const UINT PS_PerFrameCB = 0u;
		static const UINT PS_GlobalTransformsCB = 1u;
		static const UINT PS_PerCameraCB = 2u;
		static const UINT PS_PerObjectTransformCB = 3u;
		static const UINT PS_LightInputCB = 4u;
		static const UINT PS_FreeCB = 5u; // 1st available general-purpose slot
		static const UINT PS_FreeSRV = 5u; // save 0-4 for materials
		static const UINT PS_FreeSPL = 6u;

		// These are the first slots available for renderers
		static const UINT VS_FreeRendererCB = 0u;
		static const UINT VS_FreeRendererSRV = 0u;
		static const UINT VS_FreeRendererSPL = 0u;
		static const UINT PS_FreeRendererCB = 5u;
	};

	class RenderConstants
	{
	public:
		// These are defined in the cpp file
		static const float PI;
		static const float PI_2;
		static const float INV_PI;
		static const float INV_PI_2;
		static const float DEG_TO_RAD;
		static const float RAD_TO_DEG;
		static const std::vector<ID3D11Buffer*> NullBufferArray;
		static const std::vector<ID3D11ShaderResourceView*> NullSRVArray;
		static const std::vector<ID3D11UnorderedAccessView*> NullUAVArray;
		static const std::vector<ID3D11SamplerState*> NullSamplerArray;
	};

	class RenderPassConstants
	{
	public:
		static std::string GetRenderPassName(RenderPassType passType);
		static std::size_t GetRenderPassHash(RenderPassType passType);
		static RenderPassType GetRenderPassType(std::string passName);
	private:
		static std::unordered_map<RenderPassType, std::string> m_renderPassNameByEnum;
		static std::unordered_map<RenderPassType, std::size_t> m_renderPassHashByEnum;
		static std::unordered_map<std::size_t, RenderPassType> m_renderPassEnumByHash;
	};

}