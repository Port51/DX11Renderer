#include "Binding.h"
#include "Bindable.h"
#include <exception>
#include "RenderPass.h"

Binding::Binding(std::shared_ptr<Bindable> _pBindable)
	: pBindable(_pBindable)
{
}

void Binding::Bind(Graphics& gfx, const RenderPass& renderPass) const
{
	int bindings = 0;
	if (bindToInputAssemblerStageSlot != -1)
	{
		pBindable->BindIA(gfx, renderPass, bindToInputAssemblerStageSlot);
		bindings++;
	}
	if (bindToComputeStageSlot != -1)
	{
		pBindable->BindCS(gfx, renderPass, bindToComputeStageSlot);
		bindings++;
	}
	if (bindToVertexStageSlot != -1)
	{
		pBindable->BindVS(gfx, renderPass, bindToVertexStageSlot);
		bindings++;
	}
	if (bindToPixelStageSlot != -1)
	{
		pBindable->BindPS(gfx, renderPass, bindToPixelStageSlot);
		bindings++;
	}
	if (bindToRasterizerStageSlot)
	{
		pBindable->BindRS(gfx, renderPass);
		bindings++;
	}
	if (bindToOutputMergerStageSlot)
	{
		pBindable->BindOM(gfx, renderPass);
		bindings++;
	}
	if (bindings == 0)
	{
		throw std::runtime_error("Binding object has no binds setup! Please call something like 'SetupVSBinding()'.");
	}
}

std::shared_ptr<Bindable> Binding::GetBindable() const
{
	return pBindable;
}

Binding& Binding::SetupIABinding(UINT slot)
{
	bindToInputAssemblerStageSlot = slot;
	return *this;
}

Binding& Binding::SetupCSBinding(UINT slot)
{
	bindToComputeStageSlot = slot;
	return *this;
}

Binding& Binding::SetupVSBinding(UINT slot)
{
	bindToVertexStageSlot = slot;
	return *this;
}

Binding& Binding::SetupPSBinding(UINT slot)
{
	bindToPixelStageSlot = slot;
	return *this;
}

Binding& Binding::SetupRSBinding()
{
	bindToRasterizerStageSlot = true;
	return *this;
}

Binding& Binding::SetupOMBinding()
{
	bindToOutputMergerStageSlot = true;
	return *this;
}
