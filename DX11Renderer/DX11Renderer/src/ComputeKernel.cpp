#include "pch.h"
#include "ComputeKernel.h"
#include "ComputeShader.h"
#include "GraphicsDevice.h"
#include "Buffer.h"
#include "RenderPass.h"

namespace gfx
{
	ComputeKernel::ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader)
		: m_pComputeShader(std::move(pComputeShader))
	{

	}

	ComputeKernel::~ComputeKernel()
	{
	}

	void ComputeKernel::Dispatch(const GraphicsDevice& gfx, const UINT threadCountX, const UINT threadCountY, const UINT threadCountZ)
	{
		assert(m_pComputeShader != nullptr && "Compute shader has not been created!");
		m_pComputeShader->Dispatch(gfx, threadCountX, threadCountY, threadCountZ);
	}
}