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
		: m_kernelName(kernelName), Shader(path)
	{
		if (PathEndsWithCSO(path)) CompileBytecodeBlob(gfx, path);
		else CompileBytecodeBlob(gfx, path, kernelName.c_str());

		// Create shader
		THROW_IF_FAILED(gfx.GetAdapter()->CreateComputeShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pComputeShader
		));

		if (m_pComputeShader == nullptr) THROW("Could not create shader!");

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

	void ComputeShader::Dispatch(const GraphicsDevice& gfx, const UINT threadCountX, const UINT threadCountY, const UINT threadCountZ) const
	{
		assert(m_pComputeShader != nullptr);
		auto a = m_path;
		gfx.GetContext()->CSSetShader(m_pComputeShader.Get(), nullptr, 0);

		// Determine thread counts by dividing and rounding up
		const UINT threadGroupCountX = (threadCountX + m_kernelSizeX - 1u) / m_kernelSizeX;
		const UINT threadGroupCountY = (threadCountY + m_kernelSizeY - 1u) / m_kernelSizeY;
		const UINT threadGroupCountZ = (threadCountZ + m_kernelSizeZ - 1u) / m_kernelSizeZ;

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