#include "Bindable.h"
#include <assert.h>
#include "Graphics.h"

namespace Bind
{
	std::string Bindable::GetUID() const
	{
		assert("GetUID() not set yet" && false);
		return "";
	}

	ID3D11DeviceContext* Bindable::GetContext(Graphics& gfx)
	{
		return gfx.pContext.Get();
	}

	ID3D11Device* Bindable::GetDevice(Graphics& gfx)
	{
		return gfx.pDevice.Get();
	}

	DxgiInfoManager& Bindable::GetInfoManager(Graphics& gfx)
	{
#ifndef NDEBUG
		return gfx.infoManager;
#else
		throw std::logic_error("YouFuckedUp! (tried to access gfx.infoManager in Release config)");
#endif
	}
}