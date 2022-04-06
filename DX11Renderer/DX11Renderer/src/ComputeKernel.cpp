#include "pch.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "GraphicsDevice.h"
#include "Buffer.h"
#include "RenderPass.h"

namespace gfx
{
	ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
		: m_pComputeShader(pComputeShader)
	{

	}

	void ComputeKernel::Dispatch(const GraphicsDevice& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ)
	{
		m_pComputeShader->Dispatch(gfx, threadCountX, threadCountY, threadCountZ);
	}
}