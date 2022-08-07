#include "pch.h"
#include "VertexShader.h"
#include "SharedCodex.h"
#include <typeinfo>
#include <string>
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	using namespace std::string_literals;

	u16 VertexShader::m_nextInstanceIdx = 1u; // start from 1, as 0 is reserved for "no vertex shader"

	VertexShader::VertexShader(const GraphicsDevice& gfx, const char* path)
		: VertexShader(gfx, path, "main", std::vector<std::string>())
	{}

	VertexShader::VertexShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		Shader(path, entryPoint)
	{
		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &m_pBytecodeBlob));
		THROW_IF_FAILED(gfx.GetAdapter()->CreateVertexShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pVertexShader
		));
	}

	void VertexShader::Release()
	{
		m_pBytecodeBlob.Reset();
		m_pVertexShader.Reset();
	}

	void VertexShader::BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::VS_Shader, 0u))
		{
			gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void VertexShader::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::VS_Shader, 0u);
		gfx.GetContext()->VSSetShader(nullptr, nullptr, 0u);
		REGISTER_GPU_CALL();
	}

	ID3DBlob* VertexShader::GetBytecode() const
	{
		return m_pBytecodeBlob.Get();
	}

	const u16 VertexShader::GetInstanceIdx() const
	{
		return m_instanceIdx;
	}

	std::shared_ptr<VertexShader> VertexShader::Resolve(const GraphicsDevice& gfx, const char* path)
	{
		return std::move(Codex::Resolve<VertexShader>(gfx, GenerateUID(path), path));
	}

	std::shared_ptr<VertexShader> VertexShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint)
	{
		return std::move(Resolve(gfx, path, entryPoint, std::vector<std::string>()));
	}

	std::shared_ptr<VertexShader> VertexShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
	{
		return std::move(Codex::Resolve<VertexShader>(gfx, GenerateUID(path, entryPoint, shaderDefines), path, entryPoint, shaderDefines));
	}

	std::string VertexShader::GenerateUID(const char* path)
	{
		return typeid(VertexShader).name() + "#"s + path;
	}

	std::string VertexShader::GenerateUID(const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines)
	{
		std::string uid = GenerateUID(path) + "-" + entryPoint;
		for (const auto& s : shaderDefines)
		{
			uid += ("|" + s);
		}
		return uid;
	}
}