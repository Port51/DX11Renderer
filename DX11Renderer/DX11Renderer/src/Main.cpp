#include "pch.h"
#include "App.h"

/*static uint32_t allocCount = 0;
static long allocSum = 0;
void* operator new(size_t size)
{
	allocCount++;
	allocSum += (long)size;
	return malloc(size);
}*/

#include "DX11Window.h"
#include "GraphicsDevice.h"
#include "Bindable.h"
#include "Binding.h"
#include "Buffer.h"
#include "ComputeShader.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance, // always NULL
	LPSTR lpCmdLine, // command line string
	int nCmdShow // how window should be shown (includes minimized)
)
{
	using namespace gfx;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto m_pWindow = std::make_unique<DX11Window>(400, 300, "DX11 Renderer", hInstance, nullptr);
	auto m_pGfx = std::make_unique<GraphicsDevice>(m_pWindow->GetHwnd(), 400, 300);
	while (1)
	{
		auto res = std::make_shared<Bindable>();
		for (int i = 0; i < 10; ++i)
		{
			auto test = std::make_unique<ComputeShader>(*m_pGfx, std::string("Assets\\Built\\Shaders\\HiZDepthCopy.cso"), std::string("CSMain"));
			//auto test = std::make_unique<Buffer>(D3D11_USAGE::D3D11_USAGE_DEFAULT, 0u, 8u);
			//auto test = App(1600, 900, hInstance);
		}
		Sleep(100);
	}

	return 1;


	int returnCode = 0;
	try
	{
		return gfx::App(1600, 900, hInstance).Run();
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "Exception", MB_OK | MB_ICONERROR);
		returnCode = -1;
	}
	catch (...)
	{
		MessageBox(nullptr, "Unknown exception occurred", "Unknown Exception", MB_OK | MB_ICONERROR);
		returnCode = -1;
	}

	return returnCode;
}