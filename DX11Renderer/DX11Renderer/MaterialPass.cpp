#include "MaterialPass.h"

void MaterialPass::AddBindable(std::shared_ptr<Bindable> pBindable)
{
	pBindables.emplace_back(pBindable);
}

void MaterialPass::Bind(Graphics& gfx)
{
	for (const auto& bindable : pBindables)
	{
		bindable->Bind(gfx);
	}
}
