#include "pch.h"
#include "BindingList.h"
#include "Binding.h"
#include "GraphicsDevice.h"

namespace gfx
{
	BindingList::BindingList()
	{
	}

	Binding& BindingList::AddBinding(const Binding binding)
	{
		m_bindings.emplace_back(std::move(binding));
		return m_bindings[m_bindings.size() - 1];
	}

	Binding& BindingList::AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		m_bindings.push_back(Binding(std::move(pBindable)));
		return m_bindings[m_bindings.size() - 1];
	}

	void BindingList::Bind(const GraphicsDevice& gfx) const
	{
		for (int i = 0, ct = m_bindings.size(); i < ct; ++i)
		{
			m_bindings[i].Bind(gfx);
		}
	}
}