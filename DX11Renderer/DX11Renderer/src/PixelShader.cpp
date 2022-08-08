#include "pch.h"
#include "PixelShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	using namespace std::string_literals;

	u16 PixelShader::m_nextInstanceIdx = 1u; // start from 1, as 0 is reserved for "no pixel shader"

	PixelShader::PixelShader(const GraphicsDevice& gfx, const char* path)
		: PixelShader(gfx, path, "main", std::vector<std::string>())
	{}

	PixelShader::PixelShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
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
		THROW_IF_FAILED(gfx.GetAdapter()->CreatePixelShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pPixelShader
		));

		if (m_pPixelShader == nullptr) THROW("Could not create shader!");
	}

	void PixelShader::Release()
	{
		m_pPixelShader.Reset();
	}

	void PixelShader::BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::PS_Shader, 0u))
		{
			gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void PixelShader::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_Shader, 0u);
		gfx.GetContext()->PSSetShader(nullptr, nullptr, 0u);
		REGISTER_GPU_CALL();
	}

	const u16 PixelShader::GetInstanceIdx() const
	{
		return m_instanceIdx;
	}

	std::shared_ptr<PixelShader> PixelShader::Resolve(const GraphicsDevice& gfx, const char* path)
	{
		return std::move(Codex::Resolve<PixelShader>(gfx, GenerateUID(typeid(PixelShader).name(), path), path));
	}

	std::shared_ptr<PixelShader> PixelShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint)
	{
		return std::move(Resolve(gfx, path, entryPoint, std::vector<std::string>()));
	}

	std::shared_ptr<PixelShader> PixelShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
	{
		return std::move(Codex::Resolve<PixelShader>(gfx, GenerateUID(typeid(PixelShader).name(), path, entryPoint, shaderDefines), path, entryPoint, shaderDefines));
	}

}