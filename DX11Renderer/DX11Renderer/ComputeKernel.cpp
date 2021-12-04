#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "Graphics.h"
#include "Buffer.h"

ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
	: pComputeShader(pComputeShader)
{

}

void ComputeKernel::SetConstantBuffer(UINT slot, std::shared_ptr<Buffer> pConstantBuffer)
{
	if (slot < pD3DConstantBuffers.size())
	{
		pD3DConstantBuffers[slot] = pConstantBuffer->GetD3DBuffer().Get();
	}
	else if (slot == pD3DConstantBuffers.size())
	{
		AppendConstantBuffer(pConstantBuffer);
	}
	else
	{
		throw std::runtime_error("Constant buffers set out of order");
	}
}

void ComputeKernel::AppendConstantBuffer(std::shared_ptr<Buffer> pConstantBuffer)
{
	pD3DConstantBuffers.emplace_back(pConstantBuffer->GetD3DBuffer().Get());
}

void ComputeKernel::SetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView)
{
	if (slot < pD3D_SRVs.size())
	{
		pD3D_SRVs[slot] = pShaderResourceView.Get();
	}
	else if (slot == pD3D_SRVs.size())
	{
		AppendSRV(pShaderResourceView);
	}
	else
	{
		throw std::runtime_error("SRV set out of order");
	}
}

void ComputeKernel::AppendSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView)
{
	pD3D_SRVs.emplace_back(pShaderResourceView.Get());
}

void ComputeKernel::SetUAV(UINT slot, std::shared_ptr<Buffer> pUAV)
{
	SetUAV(slot, pUAV->GetD3DUAV());
}

void ComputeKernel::AppendUAV(std::shared_ptr<Buffer> pUAV)
{
	AppendUAV(pUAV->GetD3DUAV());
}

void ComputeKernel::SetUAV(UINT slot, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV)
{
	if (slot < pD3D_UAVs.size())
	{
		pD3D_UAVs[slot] = pUAV.Get();
	}
	else if (slot == pD3D_UAVs.size())
	{
		AppendUAV(pUAV);
	}
	else
	{
		throw std::runtime_error("UAV set out of order");
	}
}

void ComputeKernel::AppendUAV(Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV)
{
	pD3D_UAVs.emplace_back(pUAV.Get());
}

void ComputeKernel::Dispatch(Graphics& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
{
	gfx.GetContext()->CSSetShader(pComputeShader->GetComputeShader().Get(), nullptr, 0);

	if (pD3DConstantBuffers.size() > 0)
	{
		gfx.GetContext()->CSSetConstantBuffers(0, pD3DConstantBuffers.size(), pD3DConstantBuffers.data());
	}

	if (pD3D_SRVs.size() > 0)
	{
		gfx.GetContext()->CSSetShaderResources(0, pD3D_SRVs.size(), pD3D_SRVs.data());
		//gfx.GetContext()->CSSetShaderResources(0, pD3D_SRVs.size(), pSRVs[0].GetAddressOf());
	}

	if (pD3D_UAVs.size() > 0)
	{
		// todo: last arg is for counters
		gfx.GetContext()->CSSetUnorderedAccessViews(0, pD3D_UAVs.size(), pD3D_UAVs.data(), NULL);
	}

	gfx.GetContext()->Dispatch(threadCountX, threadCountY, threadCountZ);
}
