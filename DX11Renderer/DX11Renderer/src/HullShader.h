#include "Shader.h"
#include "CommonHeader.h"
#include <string>

struct ID3D11HullShader;

namespace gfx
{
	class GraphicsDevice;

	class HullShader : public Shader
	{
	public:
		HullShader(const GraphicsDevice& gfx, const char* path);
		HullShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	public:
		virtual void Release() override;
		virtual void BindHS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		virtual void UnbindHS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		const u16 GetInstanceIdx() const;

	public:
		static std::shared_ptr<HullShader> Resolve(const GraphicsDevice& gfx, const char* path);
		static std::shared_ptr<HullShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint);
		static std::shared_ptr<HullShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	protected:
		static u16 m_nextInstanceIdx;
		u16 m_instanceIdx;
		ComPtr<ID3D11HullShader> m_pHullShader;
	};
}