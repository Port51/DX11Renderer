#include "ComputeShader.h"
#include "GraphicsThrowMacros.h"
#include "SharedCodex.h"

using namespace std::string_literals;

ComputeShader::ComputeShader(Graphics& gfx, const std::string& path, const std::string& kernelName)
	: path(path), kernelName(kernelName)
{
	SETUP_LOGGING(gfx);

	std::wstring wide{ path.begin(), path.end() }; // convert to wide for file read <-- won't work for special characters
	GFX_THROW_INFO(D3DReadFileToBlob(wide.c_str(), &pBytecodeBlob));
	GFX_THROW_INFO(gfx.GetDevice()->CreateComputeShader(
		pBytecodeBlob->GetBufferPointer(),
		pBytecodeBlob->GetBufferSize(),
		nullptr,
		&pComputeShader
	));

	/*
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	LPCSTR profile = (gfx.GetDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	// todo: options for using pre-compiled blobs, or macros
	ID3DBlob* errorBlob = nullptr;
	std::wstring wide{ path.begin(), path.end() }; // convert to wide for file read <-- won't work for special characters
	hr = D3DCompileFromFile(wide.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		kernelName.c_str(), profile,
		flags, 0, &pBytecodeBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}*/

}

void ComputeShader::Dispatch(Graphics & gfx, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) const
{
	gfx.GetContext()->CSSetShader(pComputeShader.Get(), nullptr, 0);
	gfx.GetContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader::GetComputeShader() const
{
	return pComputeShader;
}

ID3DBlob* ComputeShader::GetBytecode() const
{
	return pBytecodeBlob.Get();
}

std::shared_ptr<ComputeShader> ComputeShader::Resolve(Graphics& gfx, const std::string& path, const std::string& kernelName)
{
	return Bind::Codex::Resolve<ComputeShader>(gfx, GenerateUID(path, kernelName), path, kernelName);
}

std::string ComputeShader::GenerateUID(const std::string& path, const std::string& kernelName)
{
	return typeid(ComputeShader).name() + "#"s + path + "#"s + kernelName;
}
