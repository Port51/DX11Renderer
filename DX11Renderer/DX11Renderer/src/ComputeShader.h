#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	class ComputeShader : public Bindable
	{
	public:
		ComputeShader(const GraphicsDevice& gfx, const std::string& path, const std::string& kernelName);
	public:
		void Dispatch(const GraphicsDevice& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ) const;
		const ComPtr<ID3D11ComputeShader> GetComputeShader() const;
		const ID3DBlob* GetBytecode() const;
	public:
		static std::shared_ptr<ComputeShader> Resolve(const GraphicsDevice& gfx, const std::string& path, const std::string& kernelName);
	protected:
		static std::string GenerateUID(const std::string& path, const std::string& kernelName);
	protected:
		const std::string m_path;
		const std::string m_kernelName;
		UINT m_kernelSizeX;
		UINT m_kernelSizeY;
		UINT m_kernelSizeZ;
		ComPtr<ID3DBlob> m_pBytecodeBlob;
		ComPtr<ID3D11ComputeShader> m_pComputeShader;
	};
}