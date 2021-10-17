#include "WindowsInclude.h"
#include "App.h"

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
		return App().Go();
	}
	catch (const CustomException& e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONERROR);
		returnCode = -1;
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