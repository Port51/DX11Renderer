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
	pConstantBuffers[slot] = pConstantBuffer;
}

void ComputeKernel::AppendConstantBuffer(std::shared_ptr<Buffer> pConstantBuffer)
{
	pConstantBuffers.emplace_back(pConstantBuffer);
}

void ComputeKernel::Dispatch(Graphics& gfx, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
	gfx.pContext->CSSetShader(pComputeShader->GetComputeShader().Get(), nullptr, 0);

	// todo: remove this temporary code!
	pD3DConstantBuffers.resize(pConstantBuffers.size());
	for (int i = 0, ct = pD3DConstantBuffers.size(); i < ct; ++i)
	{
		pD3DConstantBuffers[i] = pConstantBuffers[i]->GetD3DBuffer().Get();
	}
	gfx.pContext->CSSetConstantBuffers(0, pD3DConstantBuffers.size(), pD3DConstantBuffers.data());

	gfx.pContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}
