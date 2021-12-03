#pragma once
#include "Bindable.h"
#include <string>
#include <wrl.h>
#include <d3d11.h>

class Graphics;

class ComputeShader : public Bindable
{
public:
	ComputeShader(Graphics& gfx, const std::string& path, const std::string& kernelName);
public:
	void Dispatch(Graphics& gfx, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) const;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> GetComputeShader() const;
	ID3DBlob* GetBytecode() const;
public:
	static std::shared_ptr<ComputeShader> Resolve(Graphics& gfx, const std::string& path, const std::string& kernelName);
protected:
	static std::string GenerateUID(const std::string& path, const std::string& kernelName);
protected:
	std::string path;
	std::string kernelName;
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> pComputeShader;
};