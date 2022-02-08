#pragma once
#include "CommonHeader.h"
#include "Graphics.h"
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
		virtual void BindSharedResources(Graphics& gfx) const;
		virtual void UnbindSharedResources(Graphics& gfx) const;
		virtual void Execute(Graphics& gfx) const;
		void Reset();
	public:
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

		std::shared_ptr<RenderTexture> GetCameraColorOut() const;
	protected:
		std::vector<Binding> bindings;
	private:
		const std::string name;
		std::vector<DrawCall> jobs; // will be replaced by render graph

		// Binds shared by everything in this render pass
		std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> pCS_CB_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> pCS_SRV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11UnorderedAccessView>>> pCS_UAV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> pCS_SPL_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> pVS_CB_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> pVS_SRV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> pVS_SPL_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11Buffer>>> pPS_CB_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>> pPS_SRV_Binds;
		std::vector<std::pair<UINT, ComPtr<ID3D11SamplerState>>> pPS_SPL_Binds;

		std::shared_ptr<RenderTexture> pCameraColorOut;

		static std::vector<ID3D11Buffer*> pNullBuffers;
		static std::vector<ID3D11ShaderResourceView*> pNullSRVs;
		static std::vector<ID3D11UnorderedAccessView*> pNullUAVs;
		static std::vector<ID3D11SamplerState*> pNullSPLs;
	};
}