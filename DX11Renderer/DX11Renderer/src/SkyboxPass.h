#pragma once
#include "RenderPass.h"
#include "CommonHeader.h"

struct ID3D11ShaderResourceView;

namespace gfx
{
	class GraphicsDevice;
	class ComputeKernel;

	class SkyboxPass : public RenderPass
	{
	public:
		enum class DepthOfFieldBokehType : u8 { DiskBokeh, HexBokeh };
	private:
		enum class DepthOfFieldSubpass : u8
		{
			PrefilterSubpass,
			FarBlurSubpass,
			NearBlurSubpass,
			CompositeSubpass,
		};
	public:
		SkyboxPass(const GraphicsDevice& gfx);
	public:
		void Execute(const GraphicsDevice& gfx, RenderState& renderState) const override;
		void SetupRenderPassDependencies(const GraphicsDevice& gfx, const ComPtr<ID3D11ShaderResourceView>& pDepthStencil, const RenderTexture& pCameraColor);
	private:
		std::unique_ptr<ComputeKernel> m_pSkyboxKernel;
	};
}
