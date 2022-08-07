#pragma once
#include "Shader.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	class ComputeShader : public Shader
	{
	public:
		ComputeShader(const GraphicsDevice& gfx, const char* path);
		ComputeShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);
		virtual void Release() override;

	public:
		void Dispatch(const GraphicsDevice& gfx, const UINT threadCountX, const UINT threadCountY, const UINT threadCountZ) const;
		const ComPtr<ID3D11ComputeShader> GetComputeShader() const;
		const ID3DBlob* GetBytecode() const;

	public:
		static std::shared_ptr<ComputeShader> Resolve(const GraphicsDevice& gfx, const char* path);
		static std::shared_ptr<ComputeShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint);
		static std::shared_ptr<ComputeShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	protected:
		static std::string GenerateUID(const char* path);
		static std::string GenerateUID(const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	protected:
		UINT m_kernelSizeX;
		UINT m_kernelSizeY;
		UINT m_kernelSizeZ;
		ComPtr<ID3D11ComputeShader> m_pComputeShader;

	};
}