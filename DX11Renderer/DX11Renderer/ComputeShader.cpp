#include "ComputeShader.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

using namespace std::string_literals;

ComputeShader::ComputeShader(Graphics& gfx, const std::string& path, const std::string& kernelName)
	: path(path), kernelName(kernelName)
{
	SETUP_LOGGING(gfx);

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	LPCSTR profile = (GetDevice(gfx)->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

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
	}

}

ID3DBlob* ComputeShader::GetBytecode() const
{
	return pBytecodeBlob.Get();
}

std::shared_ptr<ComputeShader> ComputeShader::Resolve(Graphics& gfx, const std::string& path, const std::string& kernelName)
{
	return Bind::Codex::Resolve<ComputeShader>(gfx, path, kernelName);
}

std::string ComputeShader::GenerateUID(const std::string& path, const std::string& kernelName)
{
	return typeid(ComputeShader).name() + "#"s + path + "#"s + kernelName;
}

std::string ComputeShader::GetUID() const
{
	return GenerateUID(path, kernelName);
}
