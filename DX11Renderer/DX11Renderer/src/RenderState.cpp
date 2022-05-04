#include "pch.h"
#include "RenderState.h"

namespace gfx
{

	const bool RenderState::TryUpdateIA(guid64 guid)
	{
		if (m_activeIA != guid)
		{
			m_activeIA = guid;
			return true;
		}
		return false;
	}

	const bool RenderState::TryUpdateVS(guid64 guid)
	{
		if (m_activeVS != guid)
		{
			m_activeVS = guid;
			return true;
		}
		return false;
	}

	const bool RenderState::TryUpdateGS(guid64 guid)
	{
		if (m_activeGS != guid)
		{
			m_activeGS = guid;
			return true;
		}
		return false;
	}

	const bool RenderState::TryUpdatePS(guid64 guid)
	{
		if (m_activePS != guid)
		{
			m_activePS = guid;
			return true;
		}
		return false;
	}

	const bool RenderState::TryUpdateRS(guid64 guid)
	{
		if (m_activeRS != guid)
		{
			m_activeRS = guid;
			return true;
		}
		return false;
	}

}