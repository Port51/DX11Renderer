#include "pch.h"
#include "CodexElement.h"

namespace gfx
{

	guid64 CodexElement::m_nextGuid = 1u; // leave 0u as null

	CodexElement::CodexElement()
	{
		m_guid = m_nextGuid++;
	}

	const guid64 CodexElement::GetGuid() const
	{
		return m_guid;
	}

}