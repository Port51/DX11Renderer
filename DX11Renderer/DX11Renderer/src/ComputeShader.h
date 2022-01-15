#pragma once
#include "Bindable.h"
#include "Common.h"
#include <string>
#include "DX11Include.h"

class Graphics;

class ComputeShader : public Bindable
{
public:
	ComputeShader(Graphics& gfx, const std::string& path, const std::string& kernelName);
public:
	void Dispatch(Graphics& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ) const;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> GetComputeShader() const;
	ID3DBlob* GetBytecode() const;
public:
	static std::shared_ptr<ComputeShader> Resolve(Graphics& gfx, const std::string& path, const std::string& kernelName);
protected:
	static std::string GenerateUID(const std::string& path, const std::string& kernelName);
protected:
	std::string path;
	std::string kernelName;
	UINT kernelSizeX;
	UINT kernelSizeY;
	UINT kernelSizeZ;
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> pComputeShader;
};