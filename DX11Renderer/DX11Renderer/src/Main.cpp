#include "pch.h"
#include "App.h"

static uint32_t allocCount = 0;
static long allocSum = 0;
void* operator new(size_t size)
{
	allocCount++;
	allocSum += (long)size;
	return malloc(size);
}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance, // always NULL
	LPSTR lpCmdLine, // command line string
	int nCmdShow // how window should be shown (includes minimized)
)
{
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