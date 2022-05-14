#include "CorePch.h"
#include "BaseWindow.h"
#include "InputListener.h"
#include <sstream>

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

namespace gfxcore
{
	// Window Class Stuff
	BaseWindow::WindowClass BaseWindow::WindowClass::wndClass;

	BaseWindow::WindowClass::WindowClass()
		:
		hInst(GetModuleHandle(nullptr))
	{
		// Register window class
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		//wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();
		wc.hIcon = nullptr;
		wc.hIconSm = nullptr;
		//wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));

		RegisterClassEx(&wc);
	}

	BaseWindow::WindowClass::~WindowClass()
	{
		// Unregister window class
		UnregisterClass(GetName(), GetInstance());
	}

	const LPC_STRING_TYPE BaseWindow::WindowClass::GetName()
	{
		return wndClassName;
	}

	HINSTANCE BaseWindow::WindowClass::GetInstance()
	{
		return wndClass.hInst;
	}


	// Window Stuff
	BaseWindow::BaseWindow(UINT width, UINT height, const char* name, HINSTANCE const hInstance, InputListener* const pInputListener)
		: m_width(width), m_height(height), m_pInputListener(pInputListener)
	{

		// Initialize the window class.
		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = HandleMsgSetup;
		windowClass.hInstance = hInstance;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.lpszClassName = "GfxWindowClass";

		if (!RegisterClassEx(&windowClass))
		{
			throw std::runtime_error("Failed to register window class");
		}

		RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

		// Get larger size so interior region is requested size
		if (!AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE))
		{
			throw std::runtime_error("Failed to adjust window rect");
		}

		// Create the window and store a handle to it.
		m_hWnd = CreateWindow(
			windowClass.lpszClassName,
			SafeLPC(name),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,		// We have no parent window.
			nullptr,		// We aren't using menus.
			hInstance,
			this);

		if (m_hWnd == nullptr)
		{
			throw std::runtime_error("Failed to create window. Error code: " + std::to_string(GetLastError()));
		}
	}

	BaseWindow::~BaseWindow()
	{
		DestroyWindow(m_hWnd);
	}

	void BaseWindow::Show()
	{
		// Should be called AFTER application is initialized
		ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	}

	std::optional<int> BaseWindow::ProcessMessages()
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

	void BaseWindow::SetTitle(const std::string & title)
	{
		if (SetWindowText(m_hWnd, SafeLPC(title)) == 0)
		{
			throw std::runtime_error("Failed to set window title");
		}
	}

	MouseInput & BaseWindow::GetMouse()
	{
		return m_mouse;
	}

	HWND BaseWindow::GetHwnd()
	{
		return m_hWnd;
	}

	// Setup message handler
	// note: WINAPI == CALLBACK
	LRESULT WINAPI BaseWindow::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// note: don't add any exception checks here
		// because due to how this is tied with Win32,
		// that will just crash the app rather than show an error message
		// If you need this, store the exceptions and throw later

		if (msg == WM_NCCREATE) // Sent prior to WM_CREATE when window first created
		{
			// We sent instance in lpParam
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			BaseWindow* const pWindow = static_cast<BaseWindow*>(pCreate->lpCreateParams);

			// SetWindowLongPtr() lets you set data on WinAPI side
			// including custom data (user data)

			// Set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

			// Change message proc now that setup is finished
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&BaseWindow::HandleMsgAdapter));

			// Required for window to actually be created - don't remove this!
			return DefWindowProc(hWnd, msg, wParam, lParam);

			// Forward message to window class handler
			//return pWindow->HandleMsg(hWnd, msg, wParam, lParam);
		}
		// If we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	// Main message handler - routes to instance's HandleMsg()
	// note: WINAPI == CALLBACK
	LRESULT WINAPI BaseWindow::HandleMsgAdapter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// note: don't add any exception checks here
		// because due to how this is tied with Win32,
		// that will just crash the app rather than show an error message
		// If you need this, store the exceptions and throw later

		// Retrieve ptr to window class from WinAPI
		BaseWindow* const pWindow = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		// Forward message to window class handler
		return pWindow->HandleMsg(hWnd, msg, wParam, lParam);
	}

	LRESULT BaseWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		/*case WM_CLOSE:
		{
			PostQuitMessage(0);
			// Return to avoid allowing DefWindowProc() to destroy
			// Use destructor instead (when frame loop receives 0, it will do this)
			return 0;
		}*/
		case WM_MOUSEMOVE:
		{
			m_mouse.OnMouseMove((int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			m_mouse.OnLeftButtonDown((int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			m_mouse.OnRightButtonDown((int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;
		}
		case WM_LBUTTONUP:
		{
			m_mouse.OnLeftButtonUp((int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;
		}
		case WM_RBUTTONUP:
		{
			m_mouse.OnRightButtonUp((int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;
		}
		}

		if (m_pInputListener != nullptr)
		{
			return m_pInputListener->HandleMsg(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	const BOOL BaseWindow::IsPointInWindow(const POINTS pt) const
	{
		return (pt.x >= 0 && (UINT)pt.x < m_width && pt.y >= 0 && (UINT)pt.y < m_height);
	}
}