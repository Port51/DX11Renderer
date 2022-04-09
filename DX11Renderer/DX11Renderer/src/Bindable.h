#pragma once
#include "CodexElement.h"
#include <wrl.h>

struct ID3D11DeviceContext;
struct ID3D11Device;

namespace gfx
{
	class GraphicsDevice;
	class MeshRenderer;
	class RenderPass;

	class Bindable : public CodexElement
	{
	public:
		virtual void BindIA(const GraphicsDevice& gfx, UINT slot) {}
		virtual void BindCS(const GraphicsDevice& gfx, UINT slot) {}
		virtual void BindVS(const GraphicsDevice& gfx, UINT slot) {}
		virtual void BindPS(const GraphicsDevice& gfx, UINT slot) {}
		virtual void BindRS(const GraphicsDevice& gfx) {}
		virtual void BindOM(const GraphicsDevice& gfx) {}
		virtual ~Bindable() = default;
		virtual void Release() override = 0;
	};
}