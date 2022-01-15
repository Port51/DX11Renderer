#pragma once
#include "Bindable.h"
#include <memory>
#include <string>
#include <wrl.h>
#include "DX11Include.h"

class Graphics;

class Topology : public Bindable
{
public:
	Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
	void BindIA(Graphics& gfx, UINT slot) override;
public:
	static std::shared_ptr<Topology> Resolve(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
	static std::string GenerateUID(D3D11_PRIMITIVE_TOPOLOGY type);
protected:
	D3D11_PRIMITIVE_TOPOLOGY type;
};