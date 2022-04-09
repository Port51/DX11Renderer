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
		VertexShader(const GraphicsDevice& gfx, const std::string& path);
		virtual void Release() override;
		void BindVS(const GraphicsDevice& gfx, UINT slot) override;
		ID3DBlob* GetBytecode() const;
		const u16 GetInstanceIdx() const;
	public:
		static std::shared_ptr<VertexShader> Resolve(const GraphicsDevice& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
	protected:
		static u16 m_nextInstanceIdx;
		u16 m_instanceIdx;
		std::string m_path;
		ComPtr<ID3DBlob> m_pBytecodeBlob;
		ComPtr<ID3D11VertexShader> m_pVertexShader;
	};
}