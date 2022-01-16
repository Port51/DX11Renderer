#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

namespace gfx
{
	class Graphics;

	class ComputeShader : public Bindable
	{
	public:
		ComputeShader(Graphics& gfx, const std::string& path, const std::string& kernelName);
	public:
		void Dispatch(Graphics& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ) const;
		ComPtr<ID3D11ComputeShader> GetComputeShader() const;
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
		ComPtr<ID3DBlob> pBytecodeBlob;
		ComPtr<ID3D11ComputeShader> pComputeShader;
	};
}