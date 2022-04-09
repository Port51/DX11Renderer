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
		InputLayout(const GraphicsDevice& gfx, VertexLayout layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
		virtual void Release() override;
		void BindIA(const GraphicsDevice& gfx, UINT slot) override;
	public:
		static std::shared_ptr<InputLayout> Resolve(const GraphicsDevice& gfx, const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode);
		static std::string GenerateUID(const VertexLayout& layout, std::string vertexShaderName, ID3DBlob* pVertexShaderBytecode = nullptr);
	protected:
		std::string m_vertexShaderName;
		VertexLayout m_layout;
		ComPtr<ID3D11InputLayout> m_pInputLayout;
	};
}