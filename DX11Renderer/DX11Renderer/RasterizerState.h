#pragma once
#include "Bindable.h"
#include <array>
#include <wrl.h>

struct ID3D11RasterizerState;

namespace Bind
{
	class RasterizerState : public Bindable
	{
	public:
		RasterizerState(Graphics& gfx, bool twoSided);
		void Bind(Graphics& gfx, UINT slot) override;
		static std::shared_ptr<RasterizerState> Resolve(Graphics& gfx, bool twoSided);
		static std::string GenerateUID(bool twoSided);
		std::string GetUID() const override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
		bool twoSided;
	};
}