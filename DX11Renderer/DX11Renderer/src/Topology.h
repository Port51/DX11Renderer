#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <string>
#include "DX11Include.h"

namespace gfx
{
	class GraphicsDevice;

	class Topology : public Bindable
	{
	public:
		Topology(const GraphicsDevice& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
		virtual void Release() override;
		void BindIA(const GraphicsDevice& gfx, RenderState& renderState, UINT slot) override;
		void UnbindIA(const GraphicsDevice& gfx, RenderState& renderState, UINT slot) override;
	public:
		static std::shared_ptr<Topology> Resolve(const GraphicsDevice& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
		static std::string GenerateUID(D3D11_PRIMITIVE_TOPOLOGY type);
	protected:
		D3D11_PRIMITIVE_TOPOLOGY m_type;
	};
}