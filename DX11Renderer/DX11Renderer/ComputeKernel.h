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
	void SetSRV(UINT slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView);
	void AppendSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView);
	void SetUAV(UINT slot, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV);
	void AppendUAV(Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV);
	void SetUAV(UINT slot, std::shared_ptr<Buffer> pUAV);
	void AppendUAV(std::shared_ptr<Buffer> pUAV);
	void Dispatch(Graphics& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ);
private:
	std::vector<ID3D11Buffer*> pD3DConstantBuffers;
	std::vector<ID3D11ShaderResourceView*> pD3D_SRVs;
	std::vector<ID3D11UnorderedAccessView*> pD3D_UAVs;
	std::shared_ptr<ComputeShader> pComputeShader;
};