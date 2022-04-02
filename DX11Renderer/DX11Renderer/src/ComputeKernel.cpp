#include "pch.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "GraphicsDevice.h"
#include "Buffer.h"
#include "RenderPass.h"

namespace gfx
{
	ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
		: pComputeShader(pComputeShader)
	{

	}

	void ComputeKernel::Dispatch(GraphicsDevice& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
	{
		pComputeShader->Dispatch(gfx, threadCountX, threadCountY, threadCountZ);
	}
}