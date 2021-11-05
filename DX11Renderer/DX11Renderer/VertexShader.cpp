#include "VertexShader.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include <typeinfo>
#include <string>
#include "ChiliUtil.h"

using namespace std::string_literals;

VertexShader::VertexShader(Graphics& gfx, const std::string& path)
	: path(path)
{
	SETUP_LOGGING(gfx);

	std::wstring wide{ path.begin(), path.end()}; // convert to wide for file read <-- won't work for special characters
	GFX_THROW_INFO(D3DReadFileToBlob(wide.c_str(), &pBytecodeBlob));
	GFX_THROW_INFO(GetDevice(gfx)->CreateVertexShader(
		pBytecodeBlob->GetBufferPointer(),
		pBytecodeBlob->GetBufferSize(),
		nullptr,
		&pVertexShader
	));
}

void VertexShader::Bind(Graphics& gfx)
{
	GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
}

ID3DBlob* VertexShader::GetBytecode() const
{
	return pBytecodeBlob.Get();
}

///
/// If needed, will create bindable and add to bindable codex
///
std::shared_ptr<Bindable> VertexShader::Resolve(Graphics& gfx, const std::string& path)
{
	return Bind::Codex::Resolve<VertexShader>(gfx, path);
}

std::string VertexShader::GenerateUID(const std::string& path)
{
	return typeid(VertexShader).name() + "#"s + path;
}

std::string VertexShader::GetUID() const
{
	return GenerateUID(path);
}
