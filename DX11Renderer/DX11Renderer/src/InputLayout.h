#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "VertexLayout.h"
#include "DX11Include.h"
#include <string>

struct ID3D11InputLayout;

namespace gfx
{
	class GraphicsDevice;

	class InputLayout : public Bindable
	{
	public:
		InputLayout(GraphicsDevice& gfx, VertexLayout layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
		void BindIA(GraphicsDevice& gfx, UINT slot) override;
	public:
		static std::shared_ptr<InputLayout> Resolve(GraphicsDevice& gfx, const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
		static std::string GenerateUID(const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode = nullptr);
	protected:
		std::string vertexShaderName;
		VertexLayout layout;
		ComPtr<ID3D11InputLayout> pInputLayout;
	};
}