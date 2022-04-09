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

	VertexShader::VertexShader(const GraphicsDevice& gfx, const std::string& path)
		: m_instanceIdx(m_nextInstanceIdx++), // overflow is unlikely, but ok here
		m_path(path)
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

	void VertexShader::Release()
	{
		m_pBytecodeBlob.Reset();
		m_pVertexShader.Reset();
	}

	void VertexShader::BindVS(const GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
	}

	ID3DBlob* VertexShader::GetBytecode() const
	{
		return m_pBytecodeBlob.Get();
	}

	const u16 VertexShader::GetInstanceIdx() const
	{
		return m_instanceIdx;
	}

	std::shared_ptr<VertexShader> VertexShader::Resolve(const GraphicsDevice& gfx, const std::string& path)
	{
		return std::move(Codex::Resolve<VertexShader>(gfx, GenerateUID(path), path));
	}

	std::string VertexShader::GenerateUID(const std::string& path)
	{
		return typeid(VertexShader).name() + "#"s + path;
	}
}