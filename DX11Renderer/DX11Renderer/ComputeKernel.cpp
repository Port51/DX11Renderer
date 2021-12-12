#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "Graphics.h"
#include "Buffer.h"
#include "RenderPass.h"

ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
	: pComputeShader(pComputeShader)
{

}

ComputeKernel& ComputeKernel::SetConstantBuffer(UINT slot, std::shared_ptr<Buffer> pConstantBuffer)
{
	return SetConstantBuffer(slot, pConstantBuffer->GetD3DBuffer());
}

ComputeKernel& ComputeKernel::SetConstantBuffer(UINT slot, Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer)
{
	if (slot >= pD3DConstantBuffers.size())
	{
		pD3DConstantBuffers.resize(slot + 1u);
	}
	pD3DConstantBuffers[slot] = pConstantBuffer.Get();
	return *this;
}

ComputeKernel& ComputeKernel::SetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView)
{
	if (slot >= pD3D_SRVs.size())
	{
		pD3D_SRVs.resize(slot + 1u);
	}
	pD3D_SRVs[slot] = pShaderResourceView.Get();
	return *this;
}

ComputeKernel& ComputeKernel::SetUAV(UINT slot, std::shared_ptr<Buffer> pUAV)
{
	return SetUAV(slot, pUAV->GetD3DUAV());
}

ComputeKernel& ComputeKernel::SetUAV(UINT slot, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV)
{
	if (slot >= pD3D_UAVs.size())
	{
		pD3D_UAVs.resize(slot + 1u);
	}
	pD3D_UAVs[slot] = pUAV.Get();
	return *this;
}

void ComputeKernel::Dispatch(Graphics& gfx, const RenderPass& renderPass, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
{
	gfx.GetContext()->CSSetShader(pComputeShader->GetComputeShader().Get(), nullptr, 0);

	if (pD3DConstantBuffers.size() > 0)
	{
		gfx.GetContext()->CSSetConstantBuffers(renderPass.GetEndSlots().CS_CB, pD3DConstantBuffers.size(), pD3DConstantBuffers.data());
	}

	if (pD3D_SRVs.size() > 0)
	{
		gfx.GetContext()->CSSetShaderResources(renderPass.GetEndSlots().CS_SRV, pD3D_SRVs.size(), pD3D_SRVs.data());
		//gfx.GetContext()->CSSetShaderResources(0, pD3D_SRVs.size(), pSRVs[0].GetAddressOf());
	}

	if (pD3D_UAVs.size() > 0)
	{
		// todo: last arg is for counters
		gfx.GetContext()->CSSetUnorderedAccessViews(renderPass.GetEndSlots().CS_UAV, pD3D_UAVs.size(), pD3D_UAVs.data(), NULL);
	}

	gfx.GetContext()->Dispatch(threadCountX, threadCountY, threadCountZ);
}
