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

	void Shader::CompileBytecodeBlob(const GraphicsDevice& gfx, const char* path, const char* entryPoint)
    {
		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters

		// Compile shader from HLSL
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		LPCSTR profile = (gfx.GetAdapter()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
		const D3D_SHADER_MACRO defines[] =
		{
			//"EXAMPLE_DEFINE", "1",
			NULL, NULL
		};

		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile(wide.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint, profile,
			flags, 0, &m_pBytecodeBlob, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				THROW((char*)errorBlob->GetBufferPointer());
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