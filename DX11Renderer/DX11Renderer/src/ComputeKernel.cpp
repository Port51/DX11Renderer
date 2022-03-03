#include "pch.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "Graphics.h"
#include "Buffer.h"
#include "RenderPass.h"

namespace gfx
{
	ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
		: pComputeShader(pComputeShader)
	{

	}

	void ComputeKernel::Dispatch(Graphics& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
	{
		pComputeShader->Dispatch(gfx, threadCountX, threadCountY, threadCountZ);
	}
}