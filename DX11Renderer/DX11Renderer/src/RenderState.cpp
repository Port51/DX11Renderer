#include "pch.h"
#include "RenderState.h"

namespace gfx
{
	RenderState::RenderState()
	{
	}

	const bool RenderState::IsNewBinding(const guid64 guid, const RenderBindingType bindingType, const slotUINT slot)
	{
		const u16 key = GetKey(bindingType, slot);
		if (m_activeBindings.find(key) == m_activeBindings.end() || m_activeBindings.at(key) != guid)
		{
			m_activeBindings[key] = guid;
			return true;
		}
		return false;
	}

	const bool RenderState::HasBinding(const RenderBindingType bindingType, const slotUINT slot) const
	{
		const u16 key = GetKey(bindingType, slot);
		return (m_activeBindings.find(key) != m_activeBindings.end() && m_activeBindings.at(key) != NullGuid64);
	}

	void RenderState::ClearBinding(const RenderBindingType bindingType, const slotUINT slot)
	{
		const u16 key = GetKey(bindingType, slot);
		m_activeBindings[key] = NullGuid64;
	}

	const u16 RenderState::GetKey(const RenderBindingType bindingType, const slotUINT slot) const
	{
		return (((u16)bindingType) << 8u) + (u16)slot;
	}

}