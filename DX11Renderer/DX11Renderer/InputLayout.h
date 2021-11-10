#pragma once
#include "Bindable.h"
#include "VertexInclude.h"

class InputLayout : public Bindable
{
public:
	InputLayout(Graphics& gfx, VertexLayout layout, ID3DBlob* pVertexShaderBytecode);
	void Bind(Graphics& gfx) override;
	std::string GetUID() const override;
public:
	static std::shared_ptr<InputLayout> Resolve(Graphics& gfx, const VertexLayout& layout, ID3DBlob* pVertexShaderBytecode);
	static std::string GenerateUID(const VertexLayout& layout, ID3DBlob* pVertexShaderBytecode = nullptr);
protected:
	VertexLayout layout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};