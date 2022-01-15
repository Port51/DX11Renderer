#pragma once

// Currently includes Imgui 1.86

#include "./vendor/imgui/imgui.h"
#include "./vendor/imgui/imgui_impl_win32.h"
#include "./vendor/imgui/imgui_impl_dx11.h"
#include "DX11Include.h"
#include <tchar.h>

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//#include "WindowsInclude.h"
//#include "Graphics.h"
//#include "DX11Include.h"

// Forward declare this here
//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);