#pragma once
#include "Bindable.h"

namespace gfx
{
	class Shader : public Bindable
	{
	public:
		Shader(const char* path, const char* entryPoint);

	protected:
		void CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path);
		void CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines, const LPCSTR profile);
		const bool PathEndsWithCSO(const char* path) const;

	protected:
		static std::string GenerateUID(const char* shaderType, const char* path);
		static std::string GenerateUID(const char* shaderType, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	protected:
		const std::string m_path;
		const std::string m_entryPoint;
		ComPtr<ID3DBlob> m_pBytecodeBlob;

	};
}