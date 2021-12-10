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

RenderPass & RenderPass::CSAppendCB(ID3D11Buffer * pCB)
{
	pCS_CB_Binds.emplace_back(pCB);
	endSlots.CS_CB++;
	return *this;
}

RenderPass & RenderPass::CSAppendSRV(ID3D11ShaderResourceView * pSRV)
{
	pCS_SRV_Binds.emplace_back(pSRV);
	endSlots.CS_SRV++;
	return *this;
}

RenderPass & RenderPass::CSAppendUAV(ID3D11UnorderedAccessView * pUAV)
{
	pCS_UAV_Binds.emplace_back(pUAV);
	endSlots.CS_UAV++;
	return *this;
}

RenderPass & RenderPass::VSAppendCB(ID3D11Buffer * pCB)
{
	pVS_CB_Binds.emplace_back(pCB);
	endSlots.VS_CB++;
	return *this;
}

RenderPass & RenderPass::VSAppendSRV(ID3D11ShaderResourceView * pSRV)
{
	pVS_SRV_Binds.emplace_back(pSRV);
	endSlots.VS_SRV++;
	return *this;
}

RenderPass & RenderPass::PSAppendCB(ID3D11Buffer * pCB)
{
	pPS_CB_Binds.emplace_back(pCB);
	endSlots.PS_CB++;
	return *this;
}

RenderPass & RenderPass::PSAppendSRV(ID3D11ShaderResourceView * pSRV)
{
	pPS_SRV_Binds.emplace_back(pSRV);
	endSlots.PS_SRV++;
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
