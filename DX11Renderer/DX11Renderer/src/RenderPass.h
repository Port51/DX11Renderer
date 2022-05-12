#pragma once
#include "CommonHeader.h"
#include "RenderConstants.h"
#include "GraphicsDevice.h"
#include "DrawCall.h"
#include <vector>
#include "CommonCbuffers.h"

namespace gfx
{
	class Binding;
	class Bindable;
	class RenderTexture;

	///
	/// Wrapper containing jobs
	///
	class RenderPass
	{
	public:
		RenderPass(const RenderPassType renderPassType);
		virtual ~RenderPass() = default;
	public:
		const RenderPassType GetRenderPassType() const;
		void EnqueueJob(DrawCall job);
		virtual void BindSharedResources(const GraphicsDevice& gfx) const;
		virtual void UnbindSharedResources(const GraphicsDevice& gfx) const;
		virtual void Execute(const GraphicsDevice& gfx) const;
		void Reset();
	public:
		RenderPass& ClearBinds();
		RenderPass& CSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource);
		RenderPass& CSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource);
		RenderPass& CSSetUAV(UINT slot, ComPtr<ID3D11UnorderedAccessView> pResource);
		RenderPass& CSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource);
		RenderPass& VSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource);
		RenderPass& VSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource);
		RenderPass& VSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource);
		RenderPass& PSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource);
		RenderPass& PSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource);
		RenderPass& PSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource);
		RenderPass& SetCameraColorOut(std::shared_ptr<RenderTexture> pCameraColor);
		Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
		Binding& AddBinding(Binding pBinding);

		const RenderTexture& GetCameraColorOut() const;
		static const std::size_t GetHash(std::string passName);
	protected:
		RenderPass& GetSubPass(const RenderPassType pass) const;
		const RenderPass& CreateSubPass(const RenderPassType pass);
		const RenderPass& CreateSubPass(const RenderPassType pass, std::unique_ptr<RenderPass> pRenderPass);
	protected:
		std::vector<Binding> m_bindings;
		const RenderPassType m_renderPassType;
		std::vector<DrawCall> m_jobs; // will be replaced by render graph

		// Binds shared by everything in this render pass
		std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> m_CS_CB_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> m_CS_SRV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11UnorderedAccessView>>> m_CS_UAV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> m_CS_SPL_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> m_VS_CB_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> m_VS_SRV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> m_VS_SPL_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> m_PS_CB_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> m_PS_SRV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> m_PS_SPL_Binds;

		std::shared_ptr<RenderTexture> m_pCameraColorOut;
		std::unordered_map<RenderPassType, std::unique_ptr<RenderPass>> m_pSubPasses;

	public:
		static std::vector<ID3D11Buffer*> m_pNullBuffers;
		static std::vector<ID3D11ShaderResourceView*> m_pNullSRVs;
		static std::vector<ID3D11UnorderedAccessView*> m_pNullUAVs;
		static std::vector<ID3D11SamplerState*> m_pNullSPLs;
	};
}