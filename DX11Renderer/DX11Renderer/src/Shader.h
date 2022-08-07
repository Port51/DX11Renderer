#pragma once
#include "Bindable.h"

namespace gfx
{
	class Shader : public Bindable
	{
	public:
		Shader(const char* path);

	protected:
		void CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path);
		void CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path, const char* entryPoint);
		const bool PathEndsWithCSO(const char* path) const;

	protected:
		const std::string m_path;
		ComPtr<ID3DBlob> m_pBytecodeBlob;

	};
}