#include "RenderPass.h"

RenderPass::RenderPass()
{
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
