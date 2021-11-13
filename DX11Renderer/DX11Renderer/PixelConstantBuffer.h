#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"
#include "WindowsInclude.h"
#include <wrl.h>
#include <d3d11.h>

class Graphics;

namespace Bind
{
	template<typename C>
	class PixelConstantBuffer : public ConstantBuffer<C>
	{
		using ConstantBuffer<C>::pConstantBuffer;
		using ConstantBuffer<C>::identifier;
		using Bindable::GetContext;
	public:
		using ConstantBuffer<C>::ConstantBuffer;

		void PixelConstantBuffer<C>::Bind(Graphics& gfx, UINT slot) override
		{
			GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
		}
		static std::shared_ptr<PixelConstantBuffer> Resolve(Graphics& gfx, std::string identifier, const C& consts)
		{
			return Bind::Codex::template Resolve<PixelConstantBuffer>(gfx, identifier, consts);
		}
		static std::shared_ptr<PixelConstantBuffer> Resolve(Graphics& gfx, std::string identifier)
		{
			return Bind::Codex::template Resolve<PixelConstantBuffer>(gfx, identifier);
		}
		static std::string GenerateUID(std::string identifier, const C&)
		{
			return GenerateUID(identifier);
		}
		static std::string GenerateUID(std::string identifier, UINT slot = 0)
		{
			using namespace std::string_literals;
			return typeid(PixelConstantBuffer).name() + "#"s + identifier + "#"s + std::to_string(slot);
		}
		std::string GetUID() const override
		{
			return GenerateUID(identifier);
		}
	};
}