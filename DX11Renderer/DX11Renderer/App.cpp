#include "App.h"
#include <sstream>

App::App()
	:
	wnd(800, 600, "The Donkey Fart Box")
{}

int App::Go()
{
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto ecode = Window::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	/*const float t = timer.Peek();
	std::ostringstream oss;
	oss << "Time: " << t;
	wnd.SetTitle(oss.str());*/

	const float c = sin(timer.Peek()) * 0.5f + 0.5f;
	wnd.Gfx().ClearBuffer(1.f, c * 0.5f, 0.f);
	wnd.Gfx().DrawTestTriangle(sin(timer.Peek()), sin(timer.Peek() * 0.5f), timer.Peek());
	wnd.Gfx().EndFrame();

	/*while (!wnd.mouse.IsEmpty())
	{
		const auto e = wnd.mouse.Read();
		if (e.GetType() == Mouse::Event::Type::Leave)
		{
			wnd.SetTitle("Gone");
		}
		else if (e.GetType() == Mouse::Event::Type::Move)
		{
			std::ostringstream oss;
			if (wnd.mouse.IsInWindow())
			{
				oss << "MP: " << e.GetPosX() << "," << e.GetPosY();
			}
			else {
				oss << "Gone!";
			}
			wnd.SetTitle(oss.str());
		}
	}*/
}