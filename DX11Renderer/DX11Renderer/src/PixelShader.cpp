#include "pch.h"
#include "PixelShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	u16 PixelShader::m_nextInstanceIdx = 1u; // start from 1, as 0 is reserved for "no pixel shader"

	PixelShader::PixelShader(const GraphicsDevice& gfx, const char* path)
		: PixelShader(gfx, path, std::string("CSMain"))
	{
		
	}

	PixelShader::PixelShader(const GraphicsDevice& gfx, const char* path, const std::string& shaderName)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		m_path(path)
	{
		/*ComPtr<ID3DBlob> pBlob;
		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &pBlob));
		THROW_IF_FAILED(gfx.GetAdapter()->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader));*/

		static std::string cso = std::string(".cso");
		bool endsWithCSO = (0 == m_path.compare(m_path.length() - cso.length(), cso.length(), cso));

		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters

		// todo: move to Shader base class
		ComPtr<ID3DBlob> pBlob;
		if (endsWithCSO)
		{
			// Read pre-compiled shader
			THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &pBlob));
		}
		else
		{
			// Compile shader from HLSL
			UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
			flags |= D3DCOMPILE_DEBUG;
#endif

			LPCSTR profile = (gfx.GetAdapter()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "ps_5_0" : "ps_4_0";
			const D3D_SHADER_MACRO defines[] =
			{
				//"EXAMPLE_DEFINE", "1",
				NULL, NULL
			};

			ID3DBlob* errorBlob = nullptr;
			HRESULT hr = D3DCompileFromFile(wide.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				shaderName.c_str(), profile,
				flags, 0, &pBlob, &errorBlob);

			if (FAILED(hr))
			{
				if (errorBlob)
				{
					THROW(std::string("Error loading pixel shader '") + path + std::string("'\n") + (char*)errorBlob->GetBufferPointer());
					OutputDebugStringA((char*)errorBlob->GetBufferPointer());
					errorBlob->Release();
				}
			}
		}

		assert(pBlob != nullptr);

		// Create shader
		THROW_IF_FAILED(gfx.GetAdapter()->CreatePixelShader(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&m_pPixelShader
		));

		assert(m_pPixelShader != nullptr);
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
		return std::move(Codex::Resolve<PixelShader>(gfx, GenerateUID(path, "CSMain"), path));
	}

	std::shared_ptr<PixelShader> PixelShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* shaderName)
	{
		return std::move(Codex::Resolve<PixelShader>(gfx, GenerateUID(path, shaderName), path, shaderName));
	}

	std::string PixelShader::GenerateUID(const char* path, const char* shaderName)
	{
		using namespace std::string_literals;
		return typeid(PixelShader).name() + "#"s + path + "#"s + shaderName;
	}
}