#pragma once

#include "CommonHeader.h"
#include "WindowsInclude.h"
#include "Graphics.h"
#include <optional>

namespace gfx
{
	class Window
	{
	private:
		// singleton manages registration/cleanup of window class
		class WindowClass
		{
		public:
			static const char* GetName();
			static HINSTANCE GetInstance();
		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			static constexpr const char* wndClassName = "DX11 Engine Window";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};

	public:
		Window(int width, int height, const char* name);
		virtual ~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
	public:
		static std::optional<int> ProcessMessages();
		void SetTitle(const std::string& title);
		Graphics& Gfx();
	private:
		// What we pass to Windows needs to be static
		// these get the instance
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK HandleMsgAdapter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		// Instance handle method
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		const BOOL IsPointInWindow(const POINTS pt) const;
	public:
		std::unique_ptr<Graphics> pGfx;
	private:
		int width;
		int height;
		HWND hWnd;
	};
}