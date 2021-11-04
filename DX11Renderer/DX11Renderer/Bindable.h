#pragma once
#include "Graphics.h"

class Bindable
{
public:
	virtual void Bind(Graphics& gfx) = 0;
	virtual ~Bindable() = default;
	virtual std::string GetUID() const
	{
		assert("GetUID() not set yet" && false);
		return "";
	}
protected:
	// This is a friend of graphics, but children of this are not
	// so we have these methods to help the children out
	static ID3D11DeviceContext* GetContext(Graphics& gfx);
	static ID3D11Device* GetDevice(Graphics& gfx);
	static DxgiInfoManager& GetInfoManager(Graphics& gfx);
};
