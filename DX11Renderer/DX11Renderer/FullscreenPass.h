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
	Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
	Binding& AddBinding(Binding pBinding);
private:
	std::vector<Binding> bindings;
};