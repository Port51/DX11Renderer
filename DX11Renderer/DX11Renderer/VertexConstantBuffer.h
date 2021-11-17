#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"
#include "WindowsInclude.h"
#include <wrl.h>
#include <d3d11.h>

class Graphics;

template<typename C>
class VertexConstantBuffer : public ConstantBuffer<C>
{
	// Due to template inheritance, need to do some BS to call these
	// Can use "using", this->GetContext(), or fully qualify

	// todo: switch this to fully qualified
	using ConstantBuffer<C>::pConstantBuffer;
	using ConstantBuffer<C>::identifier;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	
	void VertexConstantBuffer<C>::Bind(Graphics& gfx, UINT slot) override
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
	static std::shared_ptr<VertexConstantBuffer> Resolve(Graphics& gfx, std::string identifier, const C& consts)
	{
		return Bind::Codex::template Resolve<VertexConstantBuffer>(gfx, identifier, consts);
	}
	static std::shared_ptr<VertexConstantBuffer> Resolve(Graphics& gfx, std::string identifier)
	{
		return Bind::Codex::template Resolve<VertexConstantBuffer>(gfx, identifier);
	}
	static std::string GenerateUID(std::string identifier, const C&)
	{
		return GenerateUID(identifier);
	}
	static std::string GenerateUID(std::string identifier)
	{
		using namespace std::string_literals;
		return typeid(VertexConstantBuffer).name() + "#"s + identifier;
	}
	std::string GetUID() const override
	{
		return GenerateUID(identifier);
	}
};
