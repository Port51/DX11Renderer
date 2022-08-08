#include "pch.h"
#include "HullShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	using namespace std::string_literals;

	u16 HullShader::m_nextInstanceIdx = 1u; // start from 1, as 0 is reserved for "no pixel shader"

	HullShader::HullShader(const GraphicsDevice& gfx, const char* path)
		: HullShader(gfx, path, "main", std::vector<std::string>())
	{}

	HullShader::HullShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		Shader(path, entryPoint)
	{
		if (PathEndsWithCSO(path))
		{
			if (shaderDefines.size() > 0u) THROW("Shader defines won't be used as precompiled CSO is used!");
			CompileBytecodeBlob(gfx, path);
		}
		else CompileBytecodeBlob(gfx, path, entryPoint, shaderDefines, "ps_5_0");

		// Create shader
		THROW_IF_FAILED(gfx.GetAdapter()->CreateHullShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pHullShader
		));

		if (m_pHullShader == nullptr) THROW("Could not create shader!");
	}

	void HullShader::Release()
	{
		m_pHullShader.Reset();
	}

	void HullShader::BindHS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_Shader, 0u))
		{
			gfx.GetContext()->HSSetShader(m_pHullShader.Get(), nullptr, 0u);
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void HullShader::UnbindHS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_Shader, 0u);
		gfx.GetContext()->HSSetShader(nullptr, nullptr, 0u);
		REGISTER_GPU_CALL();
	}

	const u16 HullShader::GetInstanceIdx() const
	{
		return m_instanceIdx;
	}

	std::shared_ptr<HullShader> HullShader::Resolve(const GraphicsDevice& gfx, const char* path)
	{
		return std::move(Codex::Resolve<HullShader>(gfx, GenerateUID(typeid(HullShader).name(), path), path));
	}

	std::shared_ptr<HullShader> HullShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint)
	{
		return std::move(Resolve(gfx, path, entryPoint, std::vector<std::string>()));
	}

	std::shared_ptr<HullShader> HullShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
	{
		return std::move(Codex::Resolve<HullShader>(gfx, GenerateUID(typeid(HullShader).name(), path, entryPoint, shaderDefines), path, entryPoint, shaderDefines));
	}
}