#include "GraphicsResource.h"
#include "Graphics.h"
#include <d3d11.h>
#include <stdexcept>


ID3D11DeviceContext* GraphicsResource::GetContext(Graphics& gfx)
{
	return gfx.pContext.Get();
}

ID3D11Device* GraphicsResource::GetDevice(Graphics& gfx)
{
	return gfx.pDevice.Get();
}

DxgiInfoManager& GraphicsResource::GetInfoManager(Graphics& gfx)
{
#ifndef NDEBUG
	return gfx.infoManager;
#else
	throw std::logic_error("YouFuckedUp! (tried to access gfx.infoManager in Release config)");
#endif
}