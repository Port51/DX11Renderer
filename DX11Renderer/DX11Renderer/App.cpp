#include "App.h"
#include <memory>
#include "Surface.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
#include "ModelNode.h"
#include "GDIPlusManager.h"
#include "Imgui/imgui.h"
#include "FBXImporter.h"
#include "BindableCodex.h"
#include "VertexShader.h"
#include "Sampler.h"
#include <d3d11.h>

namespace dx = DirectX;

GDIPlusManager gdipm;

App::App()
	:
	wnd(ResolutionX, ResolutionY, "DX11 Renderer"),
	light(wnd.Gfx(), dx::XMFLOAT3(4.2f, 4.2f, -5.3f), dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 9.0f),
	cam(40.0f, (float)ResolutionX / (float)ResolutionY)
{
	fc = std::make_unique<FrameCommander>(wnd.Gfx());

	std::string fn;
	dx::XMMATRIX modelTransform;

	// temporary...
	int select = 0;
	switch (select)
	{
	case 0:
		fn = std::string("Assets\\Models\\Head.asset");
		modelTransform = 
			dx::XMMatrixRotationY(3.1415f)
			* dx::XMMatrixTranslation(0.f, -4.7f, 0.f)
			* dx::XMMatrixScaling(5.f, 5.f, 5.f); // hack to keep model centered
		break;
	case 1:
		fn = std::string("Models\\SceneGraphTest.fbx");
		modelTransform = dx::XMMatrixIdentity();
		break;
	case 2:
		fn = std::string("Models\\TransformTest.fbx");
		modelTransform = dx::XMMatrixIdentity();
		break;
	case 3:
		fn = std::string("Models\\TransformTestMultiLevel.fbx");
		modelTransform = dx::XMMatrixIdentity();
		break;
	case 4:
		fn = std::string("Models\\TransformTestOneLevel.fbx");
		modelTransform = dx::XMMatrixIdentity();
		break;
	}
	
	auto pModelAsset = FBXImporter::LoadFBX(wnd.Gfx().log, fn.c_str(), FBXImporter::FBXNormalsMode::Import, false);
	if (pModelAsset)
	{
		wnd.Gfx().log.Info("Model loaded");
	}
	else
	{
		wnd.Gfx().log.Error("Failed to load model");
	}

	pModel0 = std::make_unique<ModelInstance>(wnd.Gfx(), pModelAsset, modelTransform);
	pModel1 = std::make_unique<ModelInstance>(wnd.Gfx(), pModelAsset, modelTransform * dx::XMMatrixTranslation(4.5f, 0.f, 0.f));

	return;
	
	/*class Factory
	{
	public:
		Factory(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAssetRef)
			: gfx(gfx), pModelAssetRef(pModelAssetRef)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			const DirectX::XMFLOAT3 materialColor = { cdist(rng),cdist(rng),cdist(rng) };

			switch (typedist(rng))
			{
			case 0:
				return std::make_unique<ModelInstance>(gfx, pModelAssetRef, materialColor, dx::XMFLOAT3(3.f, 3.f, 3.f));
				//return std::make_unique<Mesh>(gfx, materialColor, dx::XMFLOAT3(3.f, 3.f, 3.f));
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}
		}
	private:
		Graphics& gfx;
		std::unique_ptr<ModelAsset> const& pModelAssetRef;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> tessDist{ 3,30 };
	};

	Factory f(wnd.Gfx(), pModelAsset);
	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, f);*/

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
	/*for (auto& b : drawables)
	{
		b->SubmitDrawCalls(fc);
	}*/
	pModel0->SubmitDrawCalls(fc);
	pModel1->SubmitDrawCalls(fc);
	light.SubmitDrawCalls(fc);
	fc->Execute(wnd.Gfx());

	// imgui window to control simulation speed
	if (ImGui::Begin("Simulation Speed")) // checks if window open
	{
		ImGui::SliderFloat("Speed Factor", &simulationSpeed, 0.0f, 4.0f);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", wnd.kbd.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING (hold spacebar to pause)");
	}
	ImGui::End();

	// imgui windows
	cam.DrawImguiControlWindow();
	light.DrawImguiControlWindow();
	wnd.Gfx().log.DrawImguiControlWindow();

	wnd.Gfx().EndFrame();
	fc->Reset();

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