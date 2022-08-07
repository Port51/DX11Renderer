#pragma once
#include "Shader.h"
#include "CommonHeader.h"
#include <string>

struct ID3D11PixelShader;

namespace gfx
{
	class GraphicsDevice;

	class PixelShader : public Shader
	{
	public:
		PixelShader(const GraphicsDevice& gfx, const char* path);
		PixelShader(const GraphicsDevice& gfx, const char* path, const std::vector<std::string>& shaderDefines);
		virtual void Release() override;
		void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		const u16 GetInstanceIdx() const;
	public:
		static std::shared_ptr<PixelShader> Resolve(const GraphicsDevice& gfx, const char* path);
		static std::shared_ptr<PixelShader> Resolve(const GraphicsDevice& gfx, const char* path, const std::vector<std::string>& shaderDefines);
		static std::string GenerateUID(const char* path, const std::vector<std::string>& shaderDefines);
	protected:
		static u16 m_nextInstanceIdx;
		u16 m_instanceIdx;
		std::string m_path;
		ComPtr<ID3D11PixelShader> m_pPixelShader;
	};
}