#include "Shader.h"
#include "CommonHeader.h"

struct ID3D11DomainShader;

namespace gfx
{
	class GraphicsDevice;

	class DomainShader : public Shader
	{
	public:
		DomainShader(const GraphicsDevice& gfx, const char* path);
		DomainShader(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	public:
		virtual void Release() override;
		virtual void BindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		virtual void UnbindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot) override;
		const u16 GetInstanceIdx() const;

	public:
		static std::shared_ptr<DomainShader> Resolve(const GraphicsDevice& gfx, const char* path);
		static std::shared_ptr<DomainShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint);
		static std::shared_ptr<DomainShader> Resolve(const GraphicsDevice& gfx, const char* path, const char* entryPoint, const std::vector<std::string>& shaderDefines);

	protected:
		static u16 m_nextInstanceIdx;
		u16 m_instanceIdx;
		ComPtr<ID3D11DomainShader> m_pDomainShader;
	};
}