#pragma once
#include "CodexElement.h"
#include <wrl.h>

class Graphics;
class DxgiInfoManager;
class MeshRenderer;
class RenderPass;

struct ID3D11DeviceContext;
struct ID3D11Device;

class Bindable : public CodexElement
{
public:
	virtual void BindIA(Graphics& gfx, UINT slot) {}
	virtual void BindCS(Graphics& gfx, UINT slot) {}
	virtual void BindVS(Graphics& gfx, UINT slot) {}
	virtual void BindPS(Graphics& gfx, UINT slot) {}
	virtual void BindRS(Graphics& gfx) {}
	virtual void BindOM(Graphics& gfx) {}
	virtual ~Bindable() = default;
};
