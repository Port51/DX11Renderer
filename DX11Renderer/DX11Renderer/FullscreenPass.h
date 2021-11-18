#pragma once
#include "RenderPass.h"
#include <vector>
#include "Binding.h"
#include "Bindable.h"

//class Binding;
//class Bindable;
class Texture;

class FullscreenPass : public RenderPass
{
public:
	FullscreenPass(Graphics& gfx, std::shared_ptr<Texture> pInput);
	void Execute(Graphics& gfx) const override;
private:
	void AddBinding(std::shared_ptr<Bindable> pBindable, UINT slot = 0u);
	void AddBinding(Binding pBinding);
private:
	std::vector<Binding> bindings;
};