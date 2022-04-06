#pragma once

#include "BaseWindow.h"
#include "CommonHeader.h"
#include "WindowsInclude.h"
#include "MouseInput.h"
#include <optional>

namespace gfx
{
	class DX11Window : public BaseWindow
	{
	public:
		DX11Window(int width, int height, const char* name, HINSTANCE hInstance, InputListener* pInputListener);
		DX11Window(const DX11Window&) = delete;
		DX11Window& operator=(const DX11Window&) = delete;
		~DX11Window() override;
	public:
	protected:
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
	};
}