#include "CommonHeader.h"

namespace gfx
{

	// Helper class for minimizing state changes during draw calls
	class RenderState
	{
	public:
		const bool IsNewIA(guid64 guid);
		const bool IsNewVS(guid64 guid);
		const bool IsNewGS(guid64 guid);
		const bool IsNewPS(guid64 guid);
		const bool IsNewRS(guid64 guid);
	private:
		guid64 m_activeIA = 0u;
		guid64 m_activeVS = 0u;
		guid64 m_activeGS = 0u;
		guid64 m_activePS = 0u;
		guid64 m_activeRS = 0u;
	};

}