#include "pch.h"
#include "PixelShader.h"
#include "SharedCodex.h"
#include <d3dcompiler.h>
#include "GraphicsDevice.h"

namespace gfx
{
	PixelShader::PixelShader(GraphicsDevice& gfx, const std::string& path)
		: path(path)
	{
		ComPtr<ID3DBlob> pBlob;
		std::wstring wide{ path.begin(), path.end() }; // convert to wide for file read <-- won't work for special characters
		THROW_IF_FAILED(D3DReadFileToBlob(wide.c_str(), &pBlob));
		THROW_IF_FAILED(gfx.GetAdapter()->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
	}

	void PixelShader::BindPS(GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	}

	std::shared_ptr<PixelShader> PixelShader::Resolve(GraphicsDevice& gfx, const std::string& path)
	{
		return std::move(Codex::Resolve<PixelShader>(gfx, GenerateUID(path), path));
	}

	std::string PixelShader::GenerateUID(const std::string& path)
	{
		using namespace std::string_literals;
		return typeid(PixelShader).name() + "#"s + path;
	}
}