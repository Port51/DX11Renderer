#include "pch.h"
#include "RenderPass.h"
#include "Binding.h"
#include "Bindable.h"
#include "RenderTexture.h"

namespace gfx
{
	// Statics
	std::vector<ID3D11Buffer*> RenderPass::pNullBuffers;
	std::vector<ID3D11ShaderResourceView*> RenderPass::pNullSRVs;
	std::vector<ID3D11UnorderedAccessView*> RenderPass::pNullUAVs;
	std::vector<ID3D11SamplerState*> RenderPass::pNullSPLs;

	RenderPass::RenderPass(std::string name)
		: name(name)
	{
		if (pNullBuffers.size() == 0)
		{
			// Init static info
			pNullBuffers.resize(10u, nullptr);
			pNullSRVs.resize(10u, nullptr);
			pNullUAVs.resize(10u, nullptr);
			pNullSPLs.resize(10u, nullptr);
		}
	}

	const std::string RenderPass::GetName() const
	{
		return name;
	}

	void RenderPass::EnqueueJob(DrawCall job)
	{
		jobs.push_back(job);
	}

	void RenderPass::BindSharedResources(Graphics & gfx) const
	{
		for (auto& binding : bindings)
		{
			binding.Bind(gfx);
		}

		// todo: can optimize by passing one array for each of these
		for (size_t i = 0; i < pCS_CB_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetConstantBuffers(pCS_CB_Binds[i].first, 1u, pCS_CB_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < pCS_SRV_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetShaderResources(pCS_SRV_Binds[i].first, 1u, pCS_SRV_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < pCS_UAV_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetUnorderedAccessViews(pCS_UAV_Binds[i].first, 1u, pCS_UAV_Binds[i].second.GetAddressOf(), nullptr);
		}
		for (size_t i = 0; i < pCS_SPL_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetSamplers(pCS_SPL_Binds[i].first, 1u, pCS_SPL_Binds[i].second.GetAddressOf());
		}

		for (size_t i = 0; i < pVS_CB_Binds.size(); ++i)
		{
			gfx.GetContext()->VSSetConstantBuffers(pVS_CB_Binds[i].first, 1u, pVS_CB_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < pVS_SRV_Binds.size(); ++i)
		{
			gfx.GetContext()->VSSetShaderResources(pVS_SRV_Binds[i].first, 1u, pVS_SRV_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < pVS_SPL_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetSamplers(pVS_SPL_Binds[i].first, 1u, pVS_SPL_Binds[i].second.GetAddressOf());
		}

		for (size_t i = 0; i < pPS_CB_Binds.size(); ++i)
		{
			gfx.GetContext()->PSSetConstantBuffers(pPS_CB_Binds[i].first, 1u, pPS_CB_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < pPS_SRV_Binds.size(); ++i)
		{
			gfx.GetContext()->PSSetShaderResources(pPS_SRV_Binds[i].first, 1u, pPS_SRV_Binds[i].second.GetAddressOf());
		}
		for (size_t i = 0; i < pPS_SPL_Binds.size(); ++i)
		{
			gfx.GetContext()->CSSetSamplers(pPS_SPL_Binds[i].first, 1u, pPS_SPL_Binds[i].second.GetAddressOf());
		}
	}

	void RenderPass::UnbindSharedResources(Graphics & gfx) const
	{
		if (pCS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetConstantBuffers(pCS_CB_Binds[0].first, pCS_CB_Binds.size(), pNullBuffers.data());
		}
		if (pCS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetShaderResources(pCS_SRV_Binds[0].first, pCS_SRV_Binds.size(), pNullSRVs.data());
		}
		if (pCS_UAV_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetUnorderedAccessViews(pCS_UAV_Binds[0].first, pCS_UAV_Binds.size(), pNullUAVs.data(), nullptr);
		}
		if (pCS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetSamplers(pCS_SPL_Binds[0].first, pNullSPLs.size(), pNullSPLs.data());
		}

		if (pVS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetConstantBuffers(pVS_CB_Binds[0].first, pVS_CB_Binds.size(), pNullBuffers.data());
		}
		if (pVS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->VSSetShaderResources(pVS_SRV_Binds[0].first, pVS_SRV_Binds.size(), pNullSRVs.data());
		}
		if (pVS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetSamplers(pVS_SPL_Binds[0].first, pNullSPLs.size(), pNullSPLs.data());
		}

		if (pPS_CB_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetConstantBuffers(pPS_CB_Binds[0].first, pPS_CB_Binds.size(), pNullBuffers.data());
		}
		if (pPS_SRV_Binds.size() > 0)
		{
			gfx.GetContext()->PSSetShaderResources(pPS_SRV_Binds[0].first, pPS_SRV_Binds.size(), pNullSRVs.data());
		}
		if (pPS_SPL_Binds.size() > 0)
		{
			gfx.GetContext()->CSSetSamplers(pPS_SPL_Binds[0].first, pNullSPLs.size(), pNullSPLs.data());
		}
	}

	void RenderPass::Execute(Graphics & gfx) const
	{
		for (const auto& j : jobs)
		{
			j.Execute(gfx);
		}
	}

	void RenderPass::Reset()
	{
		jobs.clear();
	}

	RenderPass & RenderPass::CSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource)
	{
		pCS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::CSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource)
	{
		pCS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::CSSetUAV(UINT slot, ComPtr<ID3D11UnorderedAccessView> pResource)
	{
		pCS_UAV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11UnorderedAccessView>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::CSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource)
	{
		pCS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::VSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource)
	{
		pVS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::VSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource)
	{
		pVS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::VSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource)
	{
		pVS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::PSSetCB(UINT slot, ComPtr<ID3D11Buffer> pResource)
	{
		pPS_CB_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11Buffer>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::PSSetSRV(UINT slot, ComPtr<ID3D11ShaderResourceView> pResource)
	{
		pPS_SRV_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::PSSetSPL(UINT slot, ComPtr<ID3D11SamplerState> pResource)
	{
		pPS_SPL_Binds.emplace_back(std::pair<UINT, ComPtr<ID3D11SamplerState>>(slot, pResource));
		return *this;
	}

	RenderPass & RenderPass::SetCameraColorOut(std::shared_ptr<RenderTexture> pCameraColor)
	{
		pCameraColorOut = pCameraColor;
		return *this;
	}

	Binding& RenderPass::AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		bindings.push_back(Binding(std::move(pBindable)));
		return bindings[bindings.size() - 1];
	}

	Binding& RenderPass::AddBinding(Binding pBinding)
	{
		bindings.push_back(std::move(pBinding));
		return bindings[bindings.size() - 1];
	}

	std::shared_ptr<RenderTexture> RenderPass::GetCameraColorOut() const
	{
		return pCameraColorOut;
	}
}