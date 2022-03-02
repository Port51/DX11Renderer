#include "pch.h"
#include "Window.h"
#include <sstream>
#include "./../resource.h"

namespace gfx
{
	// Window Class Stuff
	Window::WindowClass Window::WindowClass::wndClass;

	Window::WindowClass::WindowClass()
		:
		hInst(GetModuleHandle(nullptr))
	{
		// Register window class
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();
		wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));

		RegisterClassEx(&wc);
	}

	Window::WindowClass::~WindowClass()
	{
		// Unregister window class
		UnregisterClass(wndClassName, GetInstance());
	}

	const char* Window::WindowClass::GetName()
	{
		return wndClassName;
	}

	HINSTANCE Window::WindowClass::GetInstance()
	{
		return wndClass.hInst;
	}


	// Window Stuff
	Window::Window(int width, int height, const char* name)
		: width(width), height(height)
	{
		// Intended rect
		RECT wr;
		wr.left = 50;
		wr.right = width + wr.left;
		wr.top = 50;
		wr.bottom = height + wr.top;

		// Get larger size so interior region is requested size
		if (!AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE))
		{
			throw std::runtime_error("Failed to adjust window rect");
		}

		// Create window & get hWnd
		hWnd = CreateWindow(
			WindowClass::GetName(), name,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			nullptr, nullptr, WindowClass::GetInstance(), this // pass instance as lpParam!
		);
		if (hWnd == nullptr)
		{
			throw std::runtime_error("Failed to create window");
		}

		// Show window
		ShowWindow(hWnd, SW_SHOWDEFAULT);

		// Init ImGui
		ImGui_ImplWin32_Init(hWnd);

		// Create graphics
		pGfx = std::make_unique<Graphics>(hWnd, width, height);
	}

	Window::~Window()
	{
		ImGui_ImplWin32_Shutdown();
		DestroyWindow(hWnd);
	}

	std::optional<int> Window::ProcessMessages()
	{
		MSG msg;
		// Doesn't block
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return (int)msg.wParam;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Empty optional = not quitting app
		return {};
	}

	void Window::SetTitle(const std::string & title)
	{
		if (SetWindowText(hWnd, title.c_str()) == 0)
		{
			throw std::runtime_error("Failed to set window title");
		}
	}

	Graphics& Window::Gfx()
	{
		return *pGfx;
	}

	MouseInput & Window::GetMouse()
	{
		return mouse;
	}

	// Setup message handler
	// note: WINAPI == CALLBACK
	LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// note: don't add any exception checks here
		// because due to how this is tied with Win32,
		// that will just crash the app rather than show an error message
		// If you need this, store the exceptions and throw later

		if (msg == WM_NCCREATE) // Sent prior to WM_CREATE when window first created
		{
			// We sent instance in lpParam
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

			// SetWindowLongPtr() lets you set data on WinAPI side
			// including custom data (user data)

			// Set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

			// Change message proc now that setup is finished
			// todo: move to method
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgAdapter));

			// Forward message to window class handler
			return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
		}
		// If we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	// Main message handler - routes to instance's HandleMsg()
	// note: WINAPI == CALLBACK
	LRESULT WINAPI Window::HandleMsgAdapter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// note: don't add any exception checks here
		// because due to how this is tied with Win32,
		// that will just crash the app rather than show an error message
		// If you need this, store the exceptions and throw later

		// Retrieve ptr to window class from WinAPI
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		// Forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	// Handle Windows messages
	LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	}

	const BOOL Window::IsPointInWindow(const POINTS pt) const
	{
		return (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height);
	}
}