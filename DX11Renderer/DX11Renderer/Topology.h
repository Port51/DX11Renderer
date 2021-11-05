#pragma once
#include "Bindable.h"

class Topology : public Bindable
{
public:
	Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
	void Bind(Graphics& gfx) override;
	std::string GetUID() const noexcept override;
public:
	static std::shared_ptr<Topology> Resolve(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
	static std::string GenerateUID(D3D11_PRIMITIVE_TOPOLOGY type);
protected:
	D3D11_PRIMITIVE_TOPOLOGY type;
};