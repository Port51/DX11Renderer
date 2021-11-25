#pragma once
#include "Bindable.h"
#include "BindableCodex.h"
#include <d3d11.h>
#include <wrl.h>

namespace Bind
{
	class Stencil : public Bindable
	{
	public:
		enum class Mode
		{
			Off,
			Write,
			Mask
		};
		Stencil(Graphics& gfx, Mode mode);
		void BindOM(Graphics& gfx) override;
		static std::shared_ptr<Stencil> Resolve(Graphics& gfx, Mode mode);
		static std::string GenerateUID(Mode mode);
		std::string GetUID() const override;
	private:
		Mode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pStencil;
	};
}