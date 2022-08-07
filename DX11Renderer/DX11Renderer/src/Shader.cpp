#include "pch.h"
#include "Shader.h"
#include "GraphicsDevice.h"
#include <d3dcompiler.h>
#include <d3d11shader.h>

namespace gfx
{
    using namespace std::string_literals;

	Shader::Shader(const char* path, const char* entryPoint)
		: m_path(path), m_entryPoint(entryPoint)
	{}

	void Shader::CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path)
	{
		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters

		// Read pre-compiled shader
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &m_pBytecodeBlob));

		if (m_pBytecodeBlob == nullptr) THROW("Could not create bytecode blob!");
	}

	void Shader::CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines, const LPCSTR profile)
    {
		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters

		// Compile shader from HLSL
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		// Ref: https://gamedev.net/forums/topic/683507-d3d_shader_macro-initialization-why-does-this-work/5317541/
		std::vector<D3D_SHADER_MACRO> defines;
		for (const auto& s : shaderDefines)
		{
			D3D_SHADER_MACRO macro;
			macro.Name = s.c_str();
			macro.Definition = "1";
			defines.emplace_back(macro);
		}
		D3D_SHADER_MACRO macro;
		macro.Name = NULL;
		macro.Definition = NULL;
		defines.emplace_back(macro);

		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile(wide.c_str(), defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint, profile,
			flags, 0, &m_pBytecodeBlob, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				THROW(std::string("Shader '") + std::string(path) + std::string("' error: ") + std::string((char*)errorBlob->GetBufferPointer()));
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
		}

		if (m_pBytecodeBlob == nullptr) THROW("Could not create bytecode blob!");
    }

    const bool Shader::PathEndsWithCSO(const char* path) const
    {
        static std::string cso = std::string(".cso");
        std::string sPath = std::string(path);
        return (0 == sPath.compare(sPath.length() - cso.length(), cso.length(), cso));
    }
}