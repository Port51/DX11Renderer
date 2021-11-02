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
};
