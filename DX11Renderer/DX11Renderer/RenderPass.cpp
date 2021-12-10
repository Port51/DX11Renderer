#include "RenderPass.h"

RenderPass::RenderPass(const RenderPass & parentPass)
{
	// This is to avoid overwriting slots written by parent pass
	startSlots = parentPass.GetEndSlots();
	endSlots = startSlots;
}

void RenderPass::EnqueueJob(RenderJob job)
{
	jobs.push_back(job);
}

void RenderPass::BindSharedResources(Graphics & gfx) const
{
	if (pCS_CB_Binds.size() > 0)
	{
		gfx.GetContext()->CSSetConstantBuffers(startSlots.CS_CB, pCS_CB_Binds.size(), pCS_CB_Binds.data());
	}
	if (pCS_SRV_Binds.size() > 0)
	{
		gfx.GetContext()->CSSetShaderResources(startSlots.CS_SRV, pCS_SRV_Binds.size(), pCS_SRV_Binds.data());
	}
	if (pCS_UAV_Binds.size() > 0)
	{
		gfx.GetContext()->CSSetUnorderedAccessViews(startSlots.CS_UAV, pCS_UAV_Binds.size(), pCS_UAV_Binds.data(), nullptr);
	}

	if (pVS_CB_Binds.size() > 0)
	{
		gfx.GetContext()->VSSetConstantBuffers(startSlots.VS_CB, pVS_CB_Binds.size(), pVS_CB_Binds.data());
	}
	if (pVS_SRV_Binds.size() > 0)
	{
		gfx.GetContext()->VSSetShaderResources(startSlots.VS_SRV, pVS_SRV_Binds.size(), pVS_SRV_Binds.data());
	}

	if (pPS_CB_Binds.size() > 0)
	{
		gfx.GetContext()->PSSetConstantBuffers(startSlots.PS_SRV, pPS_CB_Binds.size(), pPS_CB_Binds.data());
	}
	if (pPS_SRV_Binds.size() > 0)
	{
		gfx.GetContext()->PSSetShaderResources(startSlots.PS_SRV, pPS_SRV_Binds.size(), pPS_SRV_Binds.data());
	}
}

void RenderPass::Execute(Graphics & gfx) const
{
	for (const auto& j : jobs)
	{
		j.Execute(gfx, *this);
	}
}

void RenderPass::Reset()
{
	jobs.clear();
}

RenderPass & RenderPass::CSSetCB(UINT slot, ID3D11Buffer * pCB)
{
	if (pCS_CB_Binds.size() <= slot)
	{
		pCS_CB_Binds.resize(slot + 1u);
	}
	pCS_CB_Binds[slot] = pCB;
	return *this;
}

RenderPass & RenderPass::CSSetSRV(UINT slot, ID3D11ShaderResourceView * pSRV)
{
	if (pCS_SRV_Binds.size() <= slot)
	{
		pCS_SRV_Binds.resize(slot + 1u);
	}
	pCS_SRV_Binds[slot] = pSRV;
	return *this;
}

RenderPass & RenderPass::CSSetUAV(UINT slot, ID3D11UnorderedAccessView * pUAV)
{
	if (pCS_UAV_Binds.size() <= slot)
	{
		pCS_UAV_Binds.resize(slot + 1u);
	}
	pCS_UAV_Binds[slot] = pUAV;
	return *this;
}

RenderPass & RenderPass::VSSetCB(UINT slot, ID3D11Buffer * pCB)
{
	if (pVS_CB_Binds.size() <= slot)
	{
		pVS_CB_Binds.resize(slot + 1u);
	}
	pVS_CB_Binds[slot] = pCB;
	return *this;
}

RenderPass & RenderPass::VSSetSRV(UINT slot, ID3D11ShaderResourceView * pSRV)
{
	if (pVS_SRV_Binds.size() <= slot)
	{
		pVS_SRV_Binds.resize(slot + 1u);
	}
	pVS_SRV_Binds[slot] = pSRV;
	return *this;
}

RenderPass & RenderPass::PSSetCB(UINT slot, ID3D11Buffer * pCB)
{
	if (pPS_CB_Binds.size() <= slot)
	{
		pPS_CB_Binds.resize(slot + 1u);
	}
	pPS_CB_Binds[slot] = pCB;
	return *this;
}

RenderPass & RenderPass::PSSetSRV(UINT slot, ID3D11ShaderResourceView * pSRV)
{
	if (pPS_SRV_Binds.size() <= slot)
	{
		pPS_SRV_Binds.resize(slot + 1u);
	}
	pPS_SRV_Binds[slot] = pSRV;
	return *this;
}

const BindSlots & RenderPass::GetStartSlots() const
{
	return startSlots;
}

const BindSlots & RenderPass::GetEndSlots() const
{
	return endSlots;
}
