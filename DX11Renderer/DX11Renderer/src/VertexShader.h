#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

namespace gfx
{
	class Graphics;

	class VertexShader : public Bindable
	{
	public:
		VertexShader(Graphics& gfx, const std::string& path);
		void BindVS(Graphics& gfx, UINT slot) override;
		ID3DBlob* GetBytecode() const;
	public:
		static std::shared_ptr<VertexShader> Resolve(Graphics& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
	protected:
		std::string path;
		ComPtr<ID3DBlob> pBytecodeBlob;
		ComPtr<ID3D11VertexShader> pVertexShader;
	};
}