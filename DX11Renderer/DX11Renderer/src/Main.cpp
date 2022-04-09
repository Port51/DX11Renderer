#include "pch.h"
#include "App.h"

// Memory leak detection (https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2022)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
#else
	#define DBG_NEW new
#endif

/*static uint32_t allocCount = 0;
static long allocSum = 0;
void* operator new(size_t size)
{
	allocCount++;
	allocSum += (long)size;
	return malloc(size);
}*/

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

	_CrtDumpMemoryLeaks();

	return returnCode;
}