#pragma once
#include "CommonHeader.h"
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;
	class ComputeShader;
	class RenderPass;

	///
	/// Wrapper class that handles dispatching of compute shaders
	///
	class ComputeKernel
	{
	public:
		ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader);
	public:
		void Dispatch(const GraphicsDevice& gfx, const UINT threadCountX, const UINT threadCountY, const UINT threadCountZ);
	private:
		const std::shared_ptr<ComputeShader> m_pComputeShader;
	};
}