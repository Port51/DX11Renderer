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
	using ConstantBuffer<C>::slot;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	
	void VertexConstantBuffer<C>::Bind(Graphics & gfx)
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};
