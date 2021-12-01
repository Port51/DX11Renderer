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
	ID3DBlob* GetBytecode() const;
	std::string GetUID() const override;
public:
	static std::shared_ptr<ComputeShader> Resolve(Graphics& gfx, const std::string& path, const std::string& kernelName);
	static std::string GenerateUID(const std::string& path, const std::string& kernelName);
protected:
	std::string path;
	std::string kernelName;
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
};