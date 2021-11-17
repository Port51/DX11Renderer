#pragma once
#include <string>
#include "WindowsInclude.h"

class Graphics;
struct ID3D11DeviceContext;
struct ID3D11Device;
class DxgiInfoManager;
class MeshRenderer;

class Bindable
{
public:
	virtual void Bind(Graphics& gfx, UINT slot) = 0;
	virtual ~Bindable() = default;
	virtual std::string GetUID() const;
	virtual void InitializeParentReference(const MeshRenderer&)
	{}
protected:
	// This is a friend of graphics, but children of this are not
	// so we have these methods to help the children out
	static ID3D11DeviceContext* GetContext(Graphics& gfx);
	static ID3D11Device* GetDevice(Graphics& gfx);
	static DxgiInfoManager& GetInfoManager(Graphics& gfx);
};
