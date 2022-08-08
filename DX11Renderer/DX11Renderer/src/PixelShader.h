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
		PixelShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	public:
		virtual void Release() override;
		virtual void BindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		virtual void UnbindPS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		const u16 GetInstanceIdx() const;

	public:
		static std::shared_ptr<PixelShader> Resolve(const GraphicsDevice& gfx, const char* path);
		static std::shared_ptr<PixelShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint);
		static std::shared_ptr<PixelShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	protected:
		static u16 m_nextInstanceIdx;
		u16 m_instanceIdx;
		ComPtr<ID3D11PixelShader> m_pPixelShader;

	};
}