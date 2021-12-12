#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "Graphics.h"
#include "Buffer.h"
#include "RenderPass.h"

ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
	: pComputeShader(pComputeShader)
{

}

void ComputeKernel::Dispatch(Graphics& gfx, const RenderPass& renderPass, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
{
	gfx.GetContext()->CSSetShader(pComputeShader->GetComputeShader().Get(), nullptr, 0);
	pComputeShader->Dispatch(gfx, threadCountX, threadCountY, threadCountZ);
}
