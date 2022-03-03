#pragma once
#include "CommonHeader.h"
#include "DX11Include.h"

namespace gfx
{
	class Graphics;
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
		void Dispatch(Graphics& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ);
	private:
		std::shared_ptr<ComputeShader> pComputeShader;
	};
}