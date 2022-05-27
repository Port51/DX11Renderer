#include "pch.h"
#include "Binding.h"
#include "Bindable.h"
#include <exception>
#include "RenderPass.h"

namespace gfx
{
	Binding::Binding(std::shared_ptr<Bindable> pBindable)
		: m_pBindable(pBindable)
	{}

	void Binding::Bind(const GraphicsDevice& gfx) const
	{
		int bindings = 0;
		if (m_IASlot != -1)
		{
			m_pBindable->BindIA(gfx, m_IASlot);
			bindings++;
		}
		if (m_CSSlot != -1)
		{
			m_pBindable->BindCS(gfx, m_CSSlot);
			bindings++;
		}
		if (m_VSSlot != -1)
		{
			m_pBindable->BindVS(gfx, m_VSSlot);
			bindings++;
		}
		if (m_PSSlot != -1)
		{
			m_pBindable->BindPS(gfx, m_PSSlot);
			bindings++;
		}
		if (m_RSBind)
		{
			m_pBindable->BindRS(gfx);
			bindings++;
		}
		if (m_OMBind)
		{
			m_pBindable->BindOM(gfx);
			bindings++;
		}
		if (bindings == 0)
		{
			THROW("Binding object has no binds setup! Please call something like 'SetupVSBinding()'.");
		}
	}

	const Bindable& Binding::GetBindable() const
	{
		return *m_pBindable.get();
	}

	Binding& Binding::SetupIABinding(UINT slot)
	{
		m_IASlot = slot;
		return *this;
	}

	Binding& Binding::SetupCSBinding(UINT slot)
	{
		m_CSSlot = slot;
		return *this;
	}

	Binding& Binding::SetupVSBinding(UINT slot)
	{
		m_VSSlot = slot;
		return *this;
	}

	Binding& Binding::SetupPSBinding(UINT slot)
	{
		m_PSSlot = slot;
		return *this;
	}

	Binding& Binding::SetupRSBinding()
	{
		m_RSBind = true;
		return *this;
	}

	Binding& Binding::SetupOMBinding()
	{
		m_OMBind = true;
		return *this;
	}
}