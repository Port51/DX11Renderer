#pragma once

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

#include "CommonCoreHeader.h"
#include "WindowsInclude.h"
#include "MouseInput.h"
#include <optional>

namespace gfxcore
{
	class InputListener;

	// Handles main logic for setting up a window
	// Typically, you should create child classes (like DX12Window) which setup Imgui
	class BaseWindow
	{
	private:
		// singleton manages registration/cleanup of window class
		class WindowClass
		{
		public:
			static const LPC_STRING_TYPE GetName();
			static HINSTANCE GetInstance();
		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			
			static WindowClass wndClass;
			HINSTANCE hInst;
#ifdef UNICODE
			static constexpr LPCWSTR wndClassName = L"GraphicsWindowClass";
#else
			static constexpr LPCSTR wndClassName = "GraphicsWindowClass";
#endif
		};

	public:
		BaseWindow(UINT width, UINT height, const char* name, HINSTANCE const hInstance, InputListener* const pInputListener);
		virtual ~BaseWindow();
		BaseWindow(const BaseWindow&) = delete;
		BaseWindow& operator=(const BaseWindow&) = delete;
	public:
		void Show();
		static std::optional<int> ProcessMessages();
		void SetTitle(const std::string& title);
		MouseInput& GetMouse();
		HWND GetHwnd();
	private:
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK HandleMsgAdapter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	protected:
		// Instance handle method
		virtual LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		const BOOL IsPointInWindow(const POINTS pt) const;
	protected:
		InputListener* m_pInputListener;
		MouseInput m_mouse;
		UINT m_width;
		UINT m_height;
		HWND m_hWnd;
	};
}