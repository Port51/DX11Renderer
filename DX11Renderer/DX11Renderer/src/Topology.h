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
		Topology(GraphicsDevice& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
		void BindIA(GraphicsDevice& gfx, UINT slot) override;
	public:
		static std::shared_ptr<Topology> Resolve(GraphicsDevice& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
		static std::string GenerateUID(D3D11_PRIMITIVE_TOPOLOGY type);
	protected:
		D3D11_PRIMITIVE_TOPOLOGY type;
	};
}