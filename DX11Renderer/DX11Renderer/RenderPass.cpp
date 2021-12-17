#include "RenderPass.h"

// Statics
std::vector<ID3D11Buffer*> RenderPass::pNullBuffers;
std::vector<ID3D11ShaderResourceView*> RenderPass::pNullSRVs;
std::vector<ID3D11UnorderedAccessView*> RenderPass::pNullUAVs;

RenderPass::RenderPass()
{
	if (pNullBuffers.size() == 0)
	{
		// Init static info
		pNullBuffers.resize(10u, nullptr);
		pNullSRVs.resize(10u, nullptr);
		pNullUAVs.resize(10u, nullptr);
	}
}

void RenderPass::EnqueueJob(RenderJob job)
{
	jobs.push_back(job);
}

void RenderPass::BindSharedResources(Graphics & gfx) const
{
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

	for (size_t i = 0; i < pVS_CB_Binds.size(); ++i)
	{
		gfx.GetContext()->VSSetConstantBuffers(pVS_CB_Binds[i].first, 1u, pVS_CB_Binds[i].second.GetAddressOf());
	}
	for (size_t i = 0; i < pVS_SRV_Binds.size(); ++i)
	{
		gfx.GetContext()->VSSetShaderResources(pVS_SRV_Binds[i].first, 1u, pVS_SRV_Binds[i].second.GetAddressOf());
	}

	for (size_t i = 0; i < pPS_CB_Binds.size(); ++i)
	{
		gfx.GetContext()->PSSetConstantBuffers(pPS_CB_Binds[i].first, 1u, pPS_CB_Binds[i].second.GetAddressOf());
	}
	for (size_t i = 0; i < pPS_SRV_Binds.size(); ++i)
	{
		gfx.GetContext()->PSSetShaderResources(pPS_SRV_Binds[i].first, 1u, pPS_SRV_Binds[i].second.GetAddressOf());
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

	if (pVS_CB_Binds.size() > 0)
	{
		gfx.GetContext()->VSSetConstantBuffers(pVS_CB_Binds[0].first, pVS_CB_Binds.size(), pNullBuffers.data());
	}
	if (pVS_SRV_Binds.size() > 0)
	{
		gfx.GetContext()->VSSetShaderResources(pVS_SRV_Binds[0].first, pVS_SRV_Binds.size(), pNullSRVs.data());
	}

	if (pPS_CB_Binds.size() > 0)
	{
		gfx.GetContext()->PSSetConstantBuffers(pPS_CB_Binds[0].first, pPS_CB_Binds.size(), pNullBuffers.data());
	}
	if (pPS_SRV_Binds.size() > 0)
	{
		gfx.GetContext()->PSSetShaderResources(pPS_SRV_Binds[0].first, pPS_SRV_Binds.size(), pNullSRVs.data());
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

RenderPass & RenderPass::CSSetCB(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pResource)
{
	pCS_CB_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>(slot, pResource));
	return *this;
}

RenderPass & RenderPass::CSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource)
{
	pCS_SRV_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
	return *this;
}

RenderPass & RenderPass::CSSetUAV(UINT slot, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pResource)
{
	pCS_UAV_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>(slot, pResource));
	return *this;
}

RenderPass & RenderPass::VSSetCB(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pResource)
{
	pVS_CB_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>(slot, pResource));
	return *this;
}

RenderPass & RenderPass::VSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource)
{
	pVS_SRV_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
	return *this;
}

RenderPass & RenderPass::PSSetCB(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pResource)
{
	pPS_CB_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>(slot, pResource));
	return *this;
}

RenderPass & RenderPass::PSSetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource)
{
	pPS_SRV_Binds.emplace_back(std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>(slot, pResource));
	return *this;
}
