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

	Binding::~Binding()
	{}

	void Binding::BindVertexPipelineOnly(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		const guid64 guid = m_pBindable->GetGuid();

		if (m_IASlot != -1)
		{
			m_pBindable->BindIA(gfx, renderState, m_IASlot);
		}
		if (m_VSSlot != -1)
		{
			m_pBindable->BindVS(gfx, renderState, m_VSSlot);
		}
	}

	void Binding::Bind(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		const guid64 guid = m_pBindable->GetGuid();

		BindVertexPipelineOnly(gfx, renderState);

		if (m_CSSlot != -1)
		{
			m_pBindable->BindCS(gfx, renderState, m_CSSlot);
		}
		if (m_PSSlot != -1)
		{
			m_pBindable->BindPS(gfx, renderState, m_PSSlot);
		}
		if (m_RSBind)
		{
			m_pBindable->BindRS(gfx, renderState);
		}
		if (m_OMBind)
		{
			m_pBindable->BindOM(gfx, renderState);
		}
	}

	void Binding::UnbindVertexPipelineOnly(const GraphicsDevice& gfx, RenderState& renderState) const
	{
		return;
		const guid64 guid = m_pBindable->GetGuid();

		if (m_IASlot != -1)
		{
			m_pBindable->UnbindIA(gfx, renderState, m_IASlot);
		}
		if (m_VSSlot != -1)
		{
			m_pBindable->UnbindVS(gfx, renderState, m_VSSlot);
		}
	}

	void Binding::Unbind(const GraphicsDevice & gfx, RenderState & renderState) const
	{
		return;
		const guid64 guid = m_pBindable->GetGuid();

		UnbindVertexPipelineOnly(gfx, renderState);

		if (m_CSSlot != -1)
		{
			m_pBindable->UnbindCS(gfx, renderState, m_CSSlot);
		}
		if (m_PSSlot != -1)
		{
			m_pBindable->UnbindPS(gfx, renderState, m_PSSlot);
		}
		if (m_RSBind)
		{
			m_pBindable->UnbindRS(gfx, renderState);
		}
		if (m_OMBind)
		{
			m_pBindable->UnbindOM(gfx, renderState);
		}
	}

	const Bindable& Binding::GetBindable() const
	{
		return *m_pBindable.get();
	}

	Binding& Binding::SetupIABinding(const slotUINT slot)
	{
		m_IASlot = slot;
		return *this;
	}

	Binding& Binding::SetupCSBinding(const slotUINT slot)
	{
		m_CSSlot = slot;
		return *this;
	}

	Binding& Binding::SetupVSBinding(const slotUINT slot)
	{
		m_VSSlot = slot;
		return *this;
	}

	Binding& Binding::SetupPSBinding(const slotUINT slot)
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