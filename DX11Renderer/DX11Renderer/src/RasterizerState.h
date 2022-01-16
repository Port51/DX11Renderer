#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <array>

class ID3D11RasterizerState;

namespace gfx
{
	class Graphics;

	class RasterizerState : public Bindable
	{
	public:
		RasterizerState(Graphics& gfx, bool twoSided);
		void BindRS(Graphics& gfx) override;
		static std::shared_ptr<RasterizerState> Resolve(Graphics& gfx, bool twoSided);
	protected:
		static std::string GenerateUID(bool twoSided);
	protected:
		ComPtr<ID3D11RasterizerState> pRasterizer;
		bool twoSided;
	};
}