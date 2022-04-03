#include "CorePch.h"
#include "InputListener.h"

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

namespace gfxcore
{
	LRESULT InputListener::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}