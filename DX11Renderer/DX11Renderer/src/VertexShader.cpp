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

	VertexShader::VertexShader(GraphicsDevice& gfx, const std::string& path)
		: m_path(path)
	{
		std::wstring wide{ path.begin(), path.end() }; // convert to wide for file read <-- won't work for special characters
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &m_pBytecodeBlob));
		THROW_IF_FAILED(gfx.GetAdapter()->CreateVertexShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pVertexShader
		));
	}

	void VertexShader::BindVS(GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
	}

	ID3DBlob* VertexShader::GetBytecode() const
	{
		return m_pBytecodeBlob.Get();
	}

	///
	/// If needed, will create bindable and add to bindable codex
	///
	std::shared_ptr<VertexShader> VertexShader::Resolve(GraphicsDevice& gfx, const std::string& path)
	{
		return std::move(Codex::Resolve<VertexShader>(gfx, GenerateUID(path), path));
	}

	std::string VertexShader::GenerateUID(const std::string& path)
	{
		return typeid(VertexShader).name() + "#"s + path;
	}
}