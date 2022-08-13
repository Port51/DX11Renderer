#include "pch.h"
#include "DomainShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	using namespace std::string_literals;

	u16 DomainShader::m_nextInstanceIdx = 1u; // start from 1, as 0 is reserved for "no pixel shader"

	DomainShader::DomainShader(const GraphicsDevice& gfx, const char* path)
		: DomainShader(gfx, path, "main", std::vector<std::string>())
	{}

	DomainShader::DomainShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		Shader(path, entryPoint)
	{
		if (PathEndsWithCSO(path))
		{
			if (shaderDefines.size() > 0u) THROW("Shader defines won't be used as precompiled CSO is used!");
			CompileBytecodeBlob(gfx, path);
		}
		else CompileBytecodeBlob(gfx, path, entryPoint, shaderDefines, "ds_5_0");

		// Create shader
		THROW_IF_FAILED(gfx.GetAdapter()->CreateDomainShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pDomainShader
		));

		if (m_pDomainShader == nullptr) THROW("Could not create shader!");
	}

	void DomainShader::Release()
	{
		m_pDomainShader.Reset();
	}

	void DomainShader::BindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_Shader, 0u))
		{
			gfx.GetContext()->DSSetShader(m_pDomainShader.Get(), nullptr, 0u);
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void DomainShader::UnbindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_Shader, 0u);
		gfx.GetContext()->DSSetShader(nullptr, nullptr, 0u);
		REGISTER_GPU_CALL();
	}

	const u16 DomainShader::GetInstanceIdx() const
	{
		return m_instanceIdx;
	}

	std::shared_ptr<DomainShader> DomainShader::Resolve(const GraphicsDevice& gfx, const char* path)
	{
		return std::move(Codex::Resolve<DomainShader>(gfx, GenerateUID(typeid(DomainShader).name(), path), path));
	}

	std::shared_ptr<DomainShader> DomainShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint)
	{
		return std::move(Resolve(gfx, path, entryPoint, std::vector<std::string>()));
	}

	std::shared_ptr<DomainShader> DomainShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
	{
		return std::move(Codex::Resolve<DomainShader>(gfx, GenerateUID(typeid(DomainShader).name(), path, entryPoint, shaderDefines), path, entryPoint, shaderDefines));
	}
}