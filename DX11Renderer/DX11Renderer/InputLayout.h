#pragma once
#include "Bindable.h"
#include "VertexLayout.h"

class Graphics;
class VertexLayout;
struct ID3D11InputLayout;

class InputLayout : public Bindable
{
public:
	InputLayout(Graphics& gfx, VertexLayout layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
	void BindIA(Graphics& gfx, UINT slot) override;
	std::string GetUID() const override;
public:
	static std::shared_ptr<InputLayout> Resolve(Graphics& gfx, const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
	static std::string GenerateUID(const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode = nullptr);
protected:
	std::string vertexShaderName;
	VertexLayout layout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};