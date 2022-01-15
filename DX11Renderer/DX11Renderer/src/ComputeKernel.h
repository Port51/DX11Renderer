#pragma once
#include "Common.h"
#include <memory>
#include "DX11Include.h"

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
	void Dispatch(Graphics& gfx, const RenderPass& renderPass, UINT threadCountX, UINT threadCountY, UINT threadCountZ);
private:
	std::shared_ptr<ComputeShader> pComputeShader;
};