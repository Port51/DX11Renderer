#include "pch.h"
#include "App.h"
#include "MeshRenderer.h"
#include "RendererList.h"
#include "ModelInstance.h"
#include "FBXImporter.h"
#include "SharedCodex.h"
#include "VertexShader.h"
#include "Sampler.h"
#include "LightManager.h"
#include "Renderer.h"
//#include "GDIPlusManager.h"

//GDIPlusManager gdipm;

App::App(int screenWidth, int screenHeight)
	:
	wnd(screenWidth, screenHeight, "DX11 Renderer"),
	cam(40.0f, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f),
	pRendererList(std::make_shared<RendererList>())
{
	pLightManager = std::make_unique<LightManager>(wnd.Gfx(), pRendererList);

	std::string fn;
	dx::XMMATRIX modelTransform;

	// temporary...
	int select = 5;
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
	case 5:
		fn = std::string("Assets\\Models\\DefaultQuad.asset");
		modelTransform = dx::XMMatrixIdentity() * dx::XMMatrixScaling(51.f, 51.f, 51.f);
		break;
	}
	
	auto pModelAsset = FBXImporter::LoadFBX(wnd.Gfx(), fn.c_str(), FBXImporter::FBXNormalsMode::Import, false);
	if (pModelAsset)
	{
		wnd.Gfx().GetLog()->Info("Model loaded");
	}
	else
	{
		wnd.Gfx().GetLog()->Error("Failed to load model");
	}

	pModel0 = std::make_unique<ModelInstance>(wnd.Gfx(), pModelAsset, modelTransform);
	pModel1 = std::make_unique<ModelInstance>(wnd.Gfx(), pModelAsset, modelTransform * dx::XMMatrixTranslation(4.5f, 0.f, 0.f));

	pRendererList->AddModelInstance(*pModel0);
	pRendererList->AddModelInstance(*pModel1);
	pLightManager->AddLightModelsToList(*pRendererList);

	renderer = std::make_unique<Renderer>(wnd.Gfx(), pLightManager, pRendererList);

	return;
	/*class Factory
	{
	public:
		Factory(Graphics& gfx, std::unique_ptr<ModelAsset> const& pModelAssetRef)
			: gfx(gfx), pModelAssetRef(pModelAssetRef)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			const dx::XMFLOAT3 materialColor = { cdist(rng),cdist(rng),cdist(rng) };

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

	wnd.Gfx().BeginFrame();

	renderer->Execute(wnd.Gfx(), cam, pLightManager);

	if (true)
	{
		// imgui window to control simulation speed
		if (ImGui::Begin("Simulation Speed")) // checks if window open
		{
			ImGui::SliderFloat("Speed Factor", &simulationSpeed, 0.0f, 4.0f);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();

		// imgui windows
		cam.DrawImguiControlWindow();
		pLightManager->DrawImguiControlWindows();
		wnd.Gfx().GetLog()->DrawImguiControlWindow();
	}

	wnd.Gfx().EndFrame();
	renderer->Reset();

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