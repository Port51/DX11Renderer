#include "App.h"
#include "Melon.h"
#include "Pyramid.h"
#include "Box.h"
#include "SkinnedBox.h"
#include "Sheet.h"
#include <memory>
#include "GeoMath.h"
#include "Surface.h"
#include "Cylinder.h"
#include "Mesh.h"
#include "GDIPlusManager.h"
#include "Imgui/imgui.h"
#include "VertexInclude.h" // temp

namespace dx = DirectX;

GDIPlusManager gdipm;

App::App()
	:
	wnd(ResolutionX, ResolutionY, "DX11 Renderer"),
	light(wnd.Gfx(), dx::XMFLOAT3(2.2f, 3.2f, 4.2f)),
	cam(40.0f, (float)ResolutionX / (float)ResolutionY)
{
	// Quick test of VertexLayout
	/*VertexLayout vl;
	vl.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal);
	VertexBuffer vb(std::move(vl));
	vb.EmplaceBack(dx::XMFLOAT3{ 1.f, 1.f, 1.f }, dx::XMFLOAT3{ 1.f, 1.f, 1.f });
	auto pos = vb[0].Attr<VertexLayout::Position3D>();*/

	class Factory
	{
	public:
		Factory(Graphics& gfx)
			:
			gfx(gfx)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			const DirectX::XMFLOAT3 materialColor = { cdist(rng),cdist(rng),cdist(rng) };

			switch (typedist(rng))
			{
			case 0:
				return std::make_unique<Mesh>(gfx, materialColor, dx::XMFLOAT3(3.f, 3.f, 3.f));
			/*case 0:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist, materialColor
					);
			case 1:
				return std::make_unique<Cylinder>(
					gfx, rng, adist, ddist, odist,
					rdist, bdist, tessDist
					);*/
			/*case 0:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist,
					odist, rdist
					);
			case 1:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist
					);
			case 2:
				return std::make_unique<Melon>(
					gfx, rng, adist, ddist,
					odist, rdist, longdist, latdist
					);
			case 3:
				return std::make_unique<Sheet>(
					gfx, rng, adist, ddist,
					odist, rdist
					);
			case 4:
				return std::make_unique<SkinnedBox>(
					gfx, rng, adist, ddist,
					odist, rdist
					);*/
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}
		}
	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> tessDist{ 3,30 };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_int_distribution<int> latdist{ 5,20 };
		std::uniform_int_distribution<int> longdist{ 10,40 };
		std::uniform_real_distribution<float> cdist{ 0.0f,1.0f };
		std::uniform_real_distribution<float> zerodist{ 0.0f,0.0f };
		std::uniform_int_distribution<int> typedist{ 0,0 };
	};

	Factory f(wnd.Gfx());
	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, f);

	//wnd.Gfx().SetProjectionMatrix(dx::XMMatrixPerspectiveLH(1.0f, (float)ResolutionY / (float)ResolutionX, 0.5f, 40.0f));
	//wnd.Gfx().SetProjectionMatrix(dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(40.0f), (float)ResolutionX / (float)ResolutionY, 0.5f, 100.0f));
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
	auto dt = timer.Mark() * simulationSpeed;

	wnd.Gfx().BeginFrame(0.f, 0.f, 0.f);
	wnd.Gfx().SetViewMatrix(cam.GetViewMatrix());
	wnd.Gfx().SetProjectionMatrix(cam.GetProjectionMatrix());

	light.Bind(wnd.Gfx(), cam.GetViewMatrix());
	for (auto& b : drawables)
	{
		b->Update(wnd.kbd.KeyIsPressed(VK_SPACE) ? 0.f : dt);
		b->Draw(wnd.Gfx());
	}
	light.Draw(wnd.Gfx());

	// imgui window to control simulation speed
	if (ImGui::Begin("Simulation Speed")) // checks if window open
	{
		ImGui::SliderFloat("Speed Factor", &simulationSpeed, 0.0f, 4.0f);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd.kbd.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING (hold spacebar to pause)");
	}
	ImGui::End();

	// imgui windows to control camera and light
	cam.DrawImguiControlWindow();
	light.DrawImguiControlWindow();

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