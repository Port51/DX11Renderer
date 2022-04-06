#include "pch.h"
#include "DX11Window.h"
#include <sstream>
#include "./../resource.h"

namespace gfx
{
	DX11Window::DX11Window(int width, int height, const char * name, HINSTANCE hInstance, InputListener* pInputListener)
		: BaseWindow(width, height, name, hInstance, pInputListener)
	{
		// Init ImGui
		ImGui_ImplWin32_Init(m_hWnd);
	}

	DX11Window::~DX11Window()
	{
		ImGui_ImplWin32_Shutdown();
	}

	LRESULT DX11Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Imgui
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		{
			return true;
		}
		const auto& imio = ImGui::GetIO();

		return BaseWindow::HandleMsg(hWnd, msg, wParam, lParam);
	}

	// Handle Windows messages
	/*LRESULT DX11Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Imgui
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		{
			return true;
		}
		const auto& imio = ImGui::GetIO();

		switch (msg)
		{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			// Return to avoid allowing DefWindowProc() to destroy
			// Use destructor instead (when frame loop receives 0, it will do this)
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			mouse.OnMouseMove(x, y);
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			mouse.OnLeftPressed(x, y);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			mouse.OnRightPressed(x, y);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			mouse.OnLeftReleased(x, y);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			mouse.OnRightReleased(x, y);
			return 0;
		}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}*/
}