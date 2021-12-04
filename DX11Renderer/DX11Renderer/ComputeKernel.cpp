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
	if (slot < pConstantBuffers.size())
	{
		pConstantBuffers[slot] = pConstantBuffer;
	}
	else if (slot == pConstantBuffers.size())
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
	pConstantBuffers.emplace_back(pConstantBuffer);
}

void ComputeKernel::SetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView)
{
	if (slot < pSRVs.size())
	{
		pSRVs[slot] = pShaderResourceView;
	}
	else if (slot == pSRVs.size())
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
	pSRVs.emplace_back(pShaderResourceView);
}

void ComputeKernel::SetUAV(UINT slot, std::shared_ptr<Buffer> pUAV)
{
	if (slot < pUAVs.size())
	{
		pUAVs[slot] = pUAV;
	}
	else if (slot == pUAVs.size())
	{
		AppendUAV(pUAV);
	}
	else
	{
		throw std::runtime_error("UAV set out of order");
	}
}

void ComputeKernel::AppendUAV(std::shared_ptr<Buffer> pUAV)
{
	pUAVs.emplace_back(pUAV);
}

void ComputeKernel::Dispatch(Graphics& gfx, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
	gfx.GetContext()->CSSetShader(pComputeShader->GetComputeShader().Get(), nullptr, 0);

	if (pConstantBuffers.size() > 0)
	{
		// todo: remove this temporary code!
		pD3DConstantBuffers.resize(pConstantBuffers.size());
		for (int i = 0, ct = pD3DConstantBuffers.size(); i < ct; ++i)
		{
			pD3DConstantBuffers[i] = pConstantBuffers[i]->GetD3DBuffer().Get();
		}
		gfx.GetContext()->CSSetConstantBuffers(0, pD3DConstantBuffers.size(), pD3DConstantBuffers.data());
	}

	if (pSRVs.size() > 0)
	{
		// todo: remove this temporary code!
		pD3D_SRVs.resize(pSRVs.size());
		for (int i = 0, ct = pD3D_SRVs.size(); i < ct; ++i)
		{
			pD3D_SRVs[i] = pSRVs[i].Get();
		}
		gfx.GetContext()->CSSetShaderResources(0, pD3D_SRVs.size(), pD3D_SRVs.data());
		//gfx.GetContext()->CSSetShaderResources(0, pD3D_SRVs.size(), pSRVs[0].GetAddressOf());
	}

	if (pUAVs.size() > 0)
	{
		// todo: remove this temporary code!
		pD3D_UAVs.resize(pUAVs.size());
		for (int i = 0, ct = pD3D_UAVs.size(); i < ct; ++i)
		{
			pD3D_UAVs[i] = pUAVs[i]->GetD3DUAV().Get();
		}
		// todo: last arg is for counters
		gfx.GetContext()->CSSetUnorderedAccessViews(0, pD3D_UAVs.size(), pD3D_UAVs.data(), NULL);
	}

	gfx.GetContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}
