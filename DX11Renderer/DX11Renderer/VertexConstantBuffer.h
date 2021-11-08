#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"

template<typename C>
class VertexConstantBuffer : public ConstantBuffer<C>
{
	// Due to template inheritance, need to do some BS to call these
	// Can use "using", this->GetContext(), or fully qualify

	// todo: switch this to fully qualified
	using ConstantBuffer<C>::pConstantBuffer;
	using ConstantBuffer<C>::identifier;
	using ConstantBuffer<C>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	
	void VertexConstantBuffer<C>::Bind(Graphics & gfx)
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
	static std::shared_ptr<VertexConstantBuffer> Resolve(Graphics& gfx, std::string identifier, const C& consts, UINT slot = 0)
	{
		return Bind::Codex::template Resolve<VertexConstantBuffer>(gfx, identifier, consts, slot);
	}
	static std::shared_ptr<VertexConstantBuffer> Resolve(Graphics& gfx, std::string identifier, UINT slot = 0)
	{
		return Bind::Codex::template Resolve<VertexConstantBuffer>(gfx, identifier, slot);
	}
	static std::string GenerateUID(std::string identifier, const C&, UINT slot)
	{
		return GenerateUID(identifier, slot);
	}
	static std::string GenerateUID(std::string identifier, UINT slot = 0)
	{
		using namespace std::string_literals;
		return typeid(VertexConstantBuffer).name() + "#"s + identifier + "#"s + std::to_string(slot);
	}
	std::string GetUID() const noexcept override
	{
		return GenerateUID(identifier, slot);
	}
};
