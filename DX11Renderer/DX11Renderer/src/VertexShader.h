#pragma once
#include "Shader.h"
#include "CommonHeader.h"
#include <string>
#include <vector>
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	class VertexShader : public Shader
	{
	public:
		VertexShader(const GraphicsDevice& gfx, const char* path);
		VertexShader(const GraphicsDevice& gfx, const char* path, const std::vector<std::string>& shaderDefines);

	public:
		virtual void Release() override;
		void BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		ID3DBlob* GetBytecode() const;
		const u16 GetInstanceIdx() const;

	public:
		static std::shared_ptr<VertexShader> Resolve(const GraphicsDevice& gfx, const char* path);
		static std::shared_ptr<VertexShader> Resolve(const GraphicsDevice& gfx, const char* path, const std::vector<std::string>& shaderDefines);
		static std::string GenerateUID(const char* path, const std::vector<std::string>& shaderDefines);

	protected:
		static u16 m_nextInstanceIdx;
		u16 m_instanceIdx;
		ComPtr<ID3D11VertexShader> m_pVertexShader;

	};
}