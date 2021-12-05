#include "RenderPass.h"

void RenderPass::EnqueueJob(RenderJob job)
{
	jobs.push_back(job);
}

void RenderPass::BindGlobals(Graphics & gfx) const
{
	if (pGlobalVSTextureBinds.size() > 0)
	{
		gfx.GetContext()->VSSetShaderResources(0u, pGlobalVSTextureBinds.size(), pGlobalVSTextureBinds.data());
	}
	if (pGlobalPSTextureBinds.size() > 0)
	{
		gfx.GetContext()->PSSetShaderResources(0u, pGlobalPSTextureBinds.size(), pGlobalPSTextureBinds.data());
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

void RenderPass::AppendGlobalVSTextureBind(ID3D11ShaderResourceView * pSRV)
{
	pGlobalVSTextureBinds.emplace_back(pSRV);
}

void RenderPass::AppendGlobalPSTextureBind(ID3D11ShaderResourceView * pSRV)
{
	pGlobalPSTextureBinds.emplace_back(pSRV);
}
