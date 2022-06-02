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
		DX11Window(const UINT width, const UINT height, const char* name, HINSTANCE hInstance, InputListener* pInputListener);
		DX11Window(const DX11Window&) = delete;
		DX11Window& operator=(const DX11Window&) = delete;
		~DX11Window() override;
	public:
	protected:
		LRESULT HandleMsg(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam) override;
	};
}