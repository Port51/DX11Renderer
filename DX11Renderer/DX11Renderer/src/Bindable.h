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
		virtual void BindIA(GraphicsDevice& gfx, UINT slot) {}
		virtual void BindCS(GraphicsDevice& gfx, UINT slot) {}
		virtual void BindVS(GraphicsDevice& gfx, UINT slot) {}
		virtual void BindPS(GraphicsDevice& gfx, UINT slot) {}
		virtual void BindRS(GraphicsDevice& gfx) {}
		virtual void BindOM(GraphicsDevice& gfx) {}
		virtual ~Bindable() = default;
	};
}