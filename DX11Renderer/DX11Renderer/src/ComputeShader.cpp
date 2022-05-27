#include "pch.h"
#include "ComputeShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include "GraphicsDevice.h"

#pragma comment(lib,"d3dcompiler.lib")

namespace gfx
{
	using namespace std::string_literals;

	ComputeShader::ComputeShader(const GraphicsDevice & gfx, const char* path)
		: ComputeShader(gfx, path, std::string("CSMain"))
	{}

	ComputeShader::ComputeShader(const GraphicsDevice& gfx, const char* path, const std::string& kernelName)
		: m_path(path), m_kernelName(kernelName)
	{
		static std::string cso = std::string(".cso");
		bool endsWithCSO = (0 == m_path.compare(m_path.length() - cso.length(), cso.length(), cso));

		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters

		if (endsWithCSO)
		{
			// Read pre-compiled shader
			THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &m_pBytecodeBlob));
		}
		else
		{
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
				m_kernelName.c_str(), profile,
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
		}

		assert(m_pBytecodeBlob != nullptr);

		// Create shader
		THROW_IF_FAILED(gfx.GetAdapter()->CreateComputeShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pComputeShader
		));

		assert(m_pComputeShader != nullptr);

		// Read info from shader
		ID3D11ShaderReflection* pReflector = NULL;
		D3DReflect(m_pBytecodeBlob->GetBufferPointer(), m_pBytecodeBlob->GetBufferSize(),
			IID_ID3D11ShaderReflection, (void**)&pReflector);

		pReflector->GetThreadGroupSize(&m_kernelSizeX, &m_kernelSizeY, &m_kernelSizeZ);

	}

	void ComputeShader::Release()
	{
		m_pBytecodeBlob.Reset();
		m_pComputeShader.Reset();
	}

	void ComputeShader::Dispatch(const GraphicsDevice& gfx, UINT threadCountX, UINT threadCountY, UINT threadCountZ) const
	{
		assert(m_pComputeShader != nullptr);
		auto a = m_path;
		gfx.GetContext()->CSSetShader(m_pComputeShader.Get(), nullptr, 0);

		// Determine thread counts by dividing and rounding up
		UINT threadGroupCountX = (threadCountX + m_kernelSizeX - 1u) / m_kernelSizeX;
		UINT threadGroupCountY = (threadCountY + m_kernelSizeY - 1u) / m_kernelSizeY;
		UINT threadGroupCountZ = (threadCountZ + m_kernelSizeZ - 1u) / m_kernelSizeZ;

		assert(threadGroupCountX > 0u && threadGroupCountY > 0u && threadGroupCountZ > 0u && "All thread group sizes must be > 0");
		gfx.GetContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	const ComPtr<ID3D11ComputeShader> ComputeShader::GetComputeShader() const
	{
		return m_pComputeShader;
	}

	const ID3DBlob* ComputeShader::GetBytecode() const
	{
		return m_pBytecodeBlob.Get();
	}

	std::shared_ptr<ComputeShader> ComputeShader::Resolve(const GraphicsDevice& gfx, const char* path)
	{
		return std::move(ComputeShader::Resolve(gfx, path, "CSMain"));
	}

	std::shared_ptr<ComputeShader> ComputeShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* kernelName)
	{
		return std::move(Codex::Resolve<ComputeShader>(gfx, GenerateUID(path, kernelName), path, kernelName));
	}

	std::string ComputeShader::GenerateUID(const char* path, const char* kernelName)
	{
		return typeid(ComputeShader).name() + "#"s + std::string(path) + "#"s + std::string(kernelName);
	}
}