#include "Binding.h"
#include "Bindable.h"

namespace Bind
{

	Binding::Binding(std::shared_ptr<Bindable> _pBindable, UINT _slot)
		: pBindable(_pBindable), slot(_slot)
	{
	}

	void Binding::Bind(Graphics& gfx) const
	{
		pBindable->Bind(gfx, slot);
	}

	std::shared_ptr<Bindable> Binding::GetBindable() const
	{
		return pBindable;
	}
}