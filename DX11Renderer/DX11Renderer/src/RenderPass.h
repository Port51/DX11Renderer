#pragma once
#include "CommonHeader.h"
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
		RenderPass(std::string name);
		virtual ~RenderPass() = default;
	public:
		const std::string GetName() const;
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

		const std::shared_ptr<RenderTexture> GetCameraColorOut() const;
	protected:
		std::vector<Binding> m_bindings;
	private:
		const std::string m_name;
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

	public:
		static std::vector<ID3D11Buffer*> m_pNullBuffers;
		static std::vector<ID3D11ShaderResourceView*> m_pNullSRVs;
		static std::vector<ID3D11UnorderedAccessView*> m_pNullUAVs;
		static std::vector<ID3D11SamplerState*> m_pNullSPLs;
	};
}