#pragma once
#include <vector>
#include <memory>
#include <wrl.h>
#include "DX11Include.h"

class Graphics;
class ComputeShader;
class Buffer;

///
/// Wrapper class that handles binding of resources and dispatching of compute shaders
///
class ComputeKernel
{
public:
	ComputeKernel(std::shared_ptr<ComputeShader> pComputeShader);
public:
	void SetConstantBuffer(UINT slot, std::shared_ptr<Buffer> pConstantBuffer);
	void AppendConstantBuffer(std::shared_ptr<Buffer> pConstantBuffer);
	void Dispatch(Graphics& gfx, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ);
private:
	std::vector<std::shared_ptr<Buffer>> pConstantBuffers;
	std::vector<ID3D11Buffer*> pD3DConstantBuffers;
	std::shared_ptr<ComputeShader> pComputeShader;
};