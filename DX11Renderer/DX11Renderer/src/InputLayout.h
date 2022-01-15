#pragma once
#include "Bindable.h"
#include "VertexLayout.h"
#include "DX11Include.h"
#include <wrl.h>
#include <string>
#include <memory>

class Graphics;
struct ID3D11InputLayout;

class InputLayout : public Bindable
{
public:
	InputLayout(Graphics& gfx, VertexLayout layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
	void BindIA(Graphics& gfx, UINT slot) override;
public:
	static std::shared_ptr<InputLayout> Resolve(Graphics& gfx, const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
	static std::string GenerateUID(const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode = nullptr);
protected:
	std::string vertexShaderName;
	VertexLayout layout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};