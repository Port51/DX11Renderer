#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class CodexElement
	{
	public:
		CodexElement();
		virtual void Release() = 0;
		const guid64 GetGuid() const;
	protected:
		guid64 m_guid;
		static guid64 m_nextGuid;
	};
}