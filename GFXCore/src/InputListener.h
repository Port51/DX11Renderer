#pragma once

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

#include "CommonCoreHeader.h"

namespace gfxcore
{
	// Base class for the class which should receive messages from the window
	// Typically, this should be the main application class
	class InputListener
	{
	public:
		virtual LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}