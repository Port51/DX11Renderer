#include "pch.h"
#include "ComputeShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include "GraphicsDevice.h"

namespace gfx
{
	using namespace std::string_literals;

	ComputeShader::ComputeShader(GraphicsDevice& gfx, const std::string& path, const std::string& kernelName)
		: m_path(path), m_kernelName(kernelName)
	{
		std::wstring wide{ path.begin(), path.end() }; // convert to wide for file read <-- won't work for special characters
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &m_pBytecodeBlob));
		THROW_IF_FAILED(gfx.GetAdapter()->CreateComputeShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pComputeShader
		));

		// Read info from shader
		ID3D11ShaderReflection* pReflector = NULL;
		D3DReflect(m_pBytecodeBlob->GetBufferPointer(), m_pBytecodeBlob->GetBufferSize(),
			IID_ID3D11ShaderReflection, (void**)&pReflector);

		pReflector->GetThreadGroupSize(&m_kernelSizeX, &m_kernelSizeY, &m_kernelSizeZ);

		/*
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
	#endif

		// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
		LPCSTR profile = (gfx.GetDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

		const D3D_SHADER_MACRO defines[] =
		{
			"EXAMPLE_DEFINE", "1",
			NULL, NULL
		};

		// todo: options for using pre-compiled blobs, or macros
		ID3DBlob* errorBlob = nullptr;
		std::wstring wide{ path.begin(), path.end() }; // convert to wide for file read <-- won't work for special characters
		hr = D3DCompileFromFile(wide.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			kernelName.c_str(), profile,
			flags, 0, &pBytecodeBlob, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
		}*/

	}

	void ComputeShader::Dispatch(GraphicsDevice & gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ) const
	{
		gfx.GetContext()->CSSetShader(m_pComputeShader.Get(), nullptr, 0);

		// Determine thread counts by dividing and rounding up
		UINT threadGroupCountX = (threadCountX + m_kernelSizeX - 1u) / m_kernelSizeX;
		UINT threadGroupCountY = (threadCountY + m_kernelSizeY - 1u) / m_kernelSizeY;
		UINT threadGroupCountZ = (threadCountZ + m_kernelSizeZ - 1u) / m_kernelSizeZ;

		assert(threadGroupCountX > 0u && threadGroupCountY > 0u && threadGroupCountZ > 0u && "All thread group sizes must be > 0");
		gfx.GetContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	ComPtr<ID3D11ComputeShader> ComputeShader::GetComputeShader() const
	{
		return m_pComputeShader;
	}

	ID3DBlob* ComputeShader::GetBytecode() const
	{
		return m_pBytecodeBlob.Get();
	}

	std::shared_ptr<ComputeShader> ComputeShader::Resolve(GraphicsDevice& gfx, const std::string& path, const std::string& kernelName)
	{
		return std::move(Codex::Resolve<ComputeShader>(gfx, GenerateUID(path, kernelName), path, kernelName));
	}

	std::string ComputeShader::GenerateUID(const std::string& path, const std::string& kernelName)
	{
		return typeid(ComputeShader).name() + "#"s + path + "#"s + kernelName;
	}
}