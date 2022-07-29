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
		: VertexShader(gfx, path, std::string("CSMain"))
	{
		
	}

	VertexShader::VertexShader(const GraphicsDevice& gfx, const char* path, const std::string& shaderName)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		m_path(path)
	{
		static std::string cso = std::string(".cso");
		bool endsWithCSO = (0 == m_path.compare(m_path.length() - cso.length(), cso.length(), cso));

		std::wstring wide{ m_path.begin(), m_path.end() }; // convert to wide for file read <-- won't work for special characters

		// todo: move to Shader base class
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

			LPCSTR profile = (gfx.GetAdapter()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "vs_5_0" : "vs_4_0";
			const D3D_SHADER_MACRO defines[] =
			{
				//"EXAMPLE_DEFINE", "1",
				NULL, NULL
			};

			ID3DBlob* errorBlob = nullptr;
			HRESULT hr = D3DCompileFromFile(wide.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				shaderName.c_str(), profile,
				flags, 0, &m_pBytecodeBlob, &errorBlob);

			if (FAILED(hr))
			{
				if (errorBlob)
				{
					THROW(std::string("Error loading vertex shader '") + path + std::string("'\n") + (char*)errorBlob->GetBufferPointer());
					OutputDebugStringA((char*)errorBlob->GetBufferPointer());
					errorBlob->Release();
				}
			}
		}

		assert(m_pBytecodeBlob != nullptr);

		// Create shader
		THROW_IF_FAILED(gfx.GetAdapter()->CreateVertexShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pVertexShader
		));

		assert(m_pVertexShader != nullptr);
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
		}
	}

	void VertexShader::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::VS_Shader, 0u);
		gfx.GetContext()->VSSetShader(nullptr, nullptr, 0u);
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
		return std::move(Codex::Resolve<VertexShader>(gfx, GenerateUID(path, "CSMain"), path));
	}

	std::shared_ptr<VertexShader> VertexShader::Resolve(const GraphicsDevice& gfx, const char* path, const char* shaderName)
	{
		return std::move(Codex::Resolve<VertexShader>(gfx, GenerateUID(path, shaderName), path, shaderName));
	}

	std::string VertexShader::GenerateUID(const char* path, const char* shaderName)
	{
		return typeid(VertexShader).name() + "#"s + std::string(path) + "#"s + std::string(shaderName);
	}
}