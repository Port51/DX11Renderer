#include "pch.h"
#include "PixelShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	u16 PixelShader::m_nextInstanceIdx = 1u; // start from 1, as 0 is reserved for "no pixel shader"

	PixelShader::PixelShader(const GraphicsDevice& gfx, const char* path)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		m_path(path)
	{
		ComPtr<ID3DBlob> pBlob;
		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &pBlob));
		THROW_IF_FAILED(gfx.GetAdapter()->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader));
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
		}
	}

	void PixelShader::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::PS_Shader, 0u);
		gfx.GetContext()->PSSetShader(nullptr, nullptr, 0u);
	}

	const u16 PixelShader::GetInstanceIdx() const
	{
		return m_instanceIdx;
	}

	std::shared_ptr<PixelShader> PixelShader::Resolve(const GraphicsDevice& gfx, const char* path)
	{
		return std::move(Codex::Resolve<PixelShader>(gfx, GenerateUID(path), path));
	}

	std::string PixelShader::GenerateUID(const char* path)
	{
		using namespace std::string_literals;
		return typeid(PixelShader).name() + "#"s + path;
	}
}