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

	void Binding::Bind(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		const guid64 guid = m_pBindable->GetGuid();

		int bindings = 0;
		if (m_IASlot != -1)
		{
			m_pBindable->BindIA(gfx, renderState, m_IASlot);
			bindings++;
		}
		if (m_CSSlot != -1)
		{
			m_pBindable->BindCS(gfx, renderState, m_CSSlot);
			bindings++;
		}
		if (m_VSSlot != -1)
		{
			m_pBindable->BindVS(gfx, renderState, m_VSSlot);
			bindings++;
		}
		if (m_PSSlot != -1)
		{
			m_pBindable->BindPS(gfx, renderState, m_PSSlot);
			bindings++;
		}
		if (m_RSBind)
		{
			m_pBindable->BindRS(gfx, renderState);
			bindings++;
		}
		if (m_OMBind)
		{
			m_pBindable->BindOM(gfx, renderState);
			bindings++;
		}
		if (bindings == 0)
		{
			THROW("Binding object has no binds setup! Please call something like 'SetupVSBinding()'.");
		}
	}

	void Binding::Unbind(const GraphicsDevice & gfx, RenderState & renderState) const
	{
		return;
		const guid64 guid = m_pBindable->GetGuid();

		int bindings = 0;
		if (m_IASlot != -1)
		{
			m_pBindable->UnbindIA(gfx, renderState, m_IASlot);
			bindings++;
		}
		if (m_CSSlot != -1)
		{
			m_pBindable->UnbindCS(gfx, renderState, m_CSSlot);
			bindings++;
		}
		if (m_VSSlot != -1)
		{
			m_pBindable->UnbindVS(gfx, renderState, m_VSSlot);
			bindings++;
		}
		if (m_PSSlot != -1)
		{
			m_pBindable->UnbindPS(gfx, renderState, m_PSSlot);
			bindings++;
		}
		if (m_RSBind)
		{
			m_pBindable->UnbindRS(gfx, renderState);
			bindings++;
		}
		if (m_OMBind)
		{
			m_pBindable->UnbindOM(gfx, renderState);
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