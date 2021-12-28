#pragma once
#include "Bindable.h"
#include "Common.h"
#include <array>
#include <memory>

class Graphics;
class ID3D11RasterizerState;

namespace Bind
{
	class RasterizerState : public Bindable
	{
	public:
		RasterizerState(Graphics& gfx, bool twoSided);
		void BindRS(Graphics& gfx) override;
		static std::shared_ptr<RasterizerState> Resolve(Graphics& gfx, bool twoSided);
	protected:
		static std::string GenerateUID(bool twoSided);
	protected:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
		bool twoSided;
	};
}