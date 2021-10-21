#include "App.h"
#include "Box.h"
#include <memory>

App::App()
	:
	wnd(800, 600, "The Donkey Fart Box")
{
	// Create RNG
	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> adist(0.0f, 3.1415f * 2.0f);
	std::uniform_real_distribution<float> ddist(0.0f, 3.1415f * 2.0f);
	std::uniform_real_distribution<float> odist(0.0f, 3.1415f * 0.3f);
	std::uniform_real_distribution<float> rdist(6.0f, 20.0f);

	// Create boxes
	for (auto i = 0; i < 80; i++)
	{
		boxes.push_back(std::make_unique<Box>(
			wnd.Gfx(), rng, adist,
			ddist, odist, rdist
			));
	}

	// Set projection matrix
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
}

App::~App()
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
	auto dt = timer.Mark();
	//const float c = sin(timer.Peek()) * 0.5f + 0.5f;
	//wnd.Gfx().ClearBuffer(1.f, c * 0.5f, 0.f);
	wnd.Gfx().ClearBuffer(0.f, 0.f, 0.f);
	//wnd.Gfx().DrawTestTriangle(sin(timer.Peek()), sin(timer.Peek() * 0.5f), 0.f, timer.Peek());
	//wnd.Gfx().DrawTestTriangle(-sin(timer.Peek()), sin(timer.Peek() * 0.5f), 1.f, timer.Peek());
	for (auto& b : boxes)
	{
		b->Update(dt);
		b->Draw(wnd.Gfx());
	}
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