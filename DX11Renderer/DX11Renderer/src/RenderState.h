#include "CommonHeader.h"

namespace gfx
{

	// Helper class for minimizing state changes during draw calls
	class RenderState
	{
	public:
		const bool TryUpdateIA(guid64 guid);
		const bool TryUpdateVS(guid64 guid);
		const bool TryUpdateGS(guid64 guid);
		const bool TryUpdatePS(guid64 guid);
		const bool TryUpdateRS(guid64 guid);
	private:
		guid64 m_activeIA = 0u;
		guid64 m_activeVS = 0u;
		guid64 m_activeGS = 0u;
		guid64 m_activePS = 0u;
		guid64 m_activeRS = 0u;
	};

}