#include "pch.h"
#include "App.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "RendererList.h"
#include "ModelInstance.h"
#include "FBXImporter.h"
#include "SharedCodex.h"
#include "VertexShader.h"
#include "Sampler.h"
#include "LightManager.h"
#include "Renderer.h"

namespace gfx
{
	App::App(int screenWidth, int screenHeight)
		:
		pImgui(std::make_unique<ImguiManager>()),
		pWindow(std::make_unique<Window>(screenWidth, screenHeight, "DX11 Renderer")),
		pCamera(std::make_unique<Camera>(40.0f, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f)),
		pTimer(std::make_unique<Timer>()),
		pRendererList(std::make_shared<RendererList>())
	{
		pLightManager = std::make_unique<LightManager>(pWindow->Gfx(), pRendererList);

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

		auto pModelAsset = FBXImporter::LoadFBX(pWindow->Gfx(), fn.c_str(), FBXImporter::FBXNormalsMode::Import, false);
		if (pModelAsset)
		{
			pWindow->Gfx().GetLog()->Info("Model loaded");
		}
		else
		{
			pWindow->Gfx().GetLog()->Error("Failed to load model");
		}

		pModel0 = std::make_unique<ModelInstance>(pWindow->Gfx(), pModelAsset, modelTransform);
		pModel1 = std::make_unique<ModelInstance>(pWindow->Gfx(), pModelAsset, modelTransform * dx::XMMatrixTranslation(4.5f, 0.f, 0.f));

		pRendererList->AddModelInstance(*pModel0);
		pRendererList->AddModelInstance(*pModel1);
		pLightManager->AddLightModelsToList(*pRendererList);

		pRenderer = std::make_unique<Renderer>(pWindow->Gfx(), pLightManager, pRendererList);

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

		Factory f(wnd->Gfx(), pModelAsset);
		drawables.reserve(nDrawables);
		std::generate_n(std::back_inserter(drawables), nDrawables, f);*/

		//wnd->Gfx().SetProjectionMatrix(dx::XMMatrixPerspectiveLH(1.0f, (float)ResolutionY / (float)ResolutionX, 0.5f, 40.0f));
		//wnd->Gfx().SetProjectionMatrix(dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(40.0f), (float)ResolutionX / (float)ResolutionY, 0.5f, 100.0f));
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
		auto dt = pTimer->Mark() * simulationSpeed;

		pWindow->Gfx().BeginFrame();

		pRenderer->Execute(pWindow->Gfx(), pCamera, pLightManager);

		if (true)
		{
			// Imgui window to control simulation speed
			if (ImGui::Begin("Simulation Speed")) // checks if window open
			{
				ImGui::SliderFloat("Speed Factor", &simulationSpeed, 0.0f, 4.0f);
				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
			ImGui::End();

			// Imgui windows
			pCamera->DrawImguiControlWindow();
			pLightManager->DrawImguiControlWindows();
			pWindow->Gfx().GetLog()->DrawImguiControlWindow();
		}

		pWindow->Gfx().EndFrame();
		pRenderer->Reset();
	}
}