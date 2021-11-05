#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"

template<typename C>
class PixelConstantBuffer : public ConstantBuffer<C>
{
	using ConstantBuffer<C>::pConstantBuffer;
	using ConstantBuffer<C>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	
	void PixelConstantBuffer<C>::Bind(Graphics& gfx)
	{
		GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
	static std::shared_ptr<PixelConstantBuffer> Resolve(Graphics& gfx, const C& consts, UINT slot = 0)
	{
		return Bind::Codex::Resolve<PixelConstantBuffer>(gfx, consts, slot);
	}
	static std::shared_ptr<PixelConstantBuffer> Resolve(Graphics& gfx, UINT slot = 0)
	{
		return Bind::Codex::Resolve<PixelConstantBuffer>(gfx, slot);
	}
	static std::string GenerateUID(const C&, UINT slot)
	{
		return GenerateUID(slot);
	}
	static std::string GenerateUID(UINT slot = 0)
	{
		using namespace std::string_literals;
		return typeid(PixelConstantBuffer).name() + "#"s + std::to_string(slot);
	}
	std::string GetUID() const noexcept override
	{
		return GenerateUID(slot);
	}
};
