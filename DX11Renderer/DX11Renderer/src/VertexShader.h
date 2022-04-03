#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	class VertexShader : public Bindable
	{
	public:
		VertexShader(GraphicsDevice& gfx, const std::string& path);
		void BindVS(GraphicsDevice& gfx, UINT slot) override;
		ID3DBlob* GetBytecode() const;
	public:
		static std::shared_ptr<VertexShader> Resolve(GraphicsDevice& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
	protected:
		std::string m_path;
		ComPtr<ID3DBlob> m_pBytecodeBlob;
		ComPtr<ID3D11VertexShader> m_pVertexShader;
	};
}