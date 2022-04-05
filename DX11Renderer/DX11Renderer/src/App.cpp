#include "pch.h"
#include "App.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "RendererList.h"
#include "ModelInstance.h"
#include "ModelImporter.h"
#include "SharedCodex.h"
#include "VertexShader.h"
#include "Sampler.h"
#include "LightManager.h"
#include "Light.h"
#include "Renderer.h"

namespace gfx
{
	App::App(int screenWidth, int screenHeight, HINSTANCE hInstance)
		:
		m_pImgui(std::make_unique<ImguiManager>()),
		m_pCamera(std::make_unique<Camera>(40.0f, (float)screenWidth / (float)screenHeight, 0.5f, 50.0f)),
		m_pTimer(std::make_unique<Timer>()),
		m_pRendererList(std::make_shared<RendererList>())
	{
		m_pWindow = std::make_unique<DX11Window>(screenWidth, screenHeight, "DX11 Renderer", hInstance, this);

		m_pLightManager = std::make_shared<LightManager>(m_pWindow->Gfx(), m_pRendererList);

		std::string fn;
		dx::XMMATRIX modelTransform;

		// temporary...
		int select = 8;
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
		case 6:
			fn = std::string("Assets\\Models\\ShadowTestScene.asset");
			modelTransform = dx::XMMatrixRotationY(3.1415f) * dx::XMMatrixScaling(12.f, 12.f, 12.f);
			break;
		case 7:
			fn = std::string("Assets\\Models\\CascadeTestScene.asset");
			modelTransform = dx::XMMatrixRotationY(3.1415f) * dx::XMMatrixScaling(12.f, 12.f, 12.f);
			break;
		case 8:
			fn = std::string("Assets\\Models\\SimpleCastle.asset");
			modelTransform = dx::XMMatrixRotationY(3.1415f) * dx::XMMatrixScaling(1.f, 1.f, 1.f);
			break;
		}

		auto pModelAsset = ModelImporter::LoadGLTF(m_pWindow->Gfx(), fn.c_str());
		if (pModelAsset)
		{
			m_pWindow->Gfx().GetLog()->Info("Model loaded");
		}
		else
		{
			m_pWindow->Gfx().GetLog()->Error("Failed to load model");
		}

		m_pModel0 = std::make_unique<ModelInstance>(m_pWindow->Gfx(), pModelAsset, modelTransform);

		m_pRendererList->AddModelInstance(*m_pModel0);
		m_pLightManager->AddLightModelsToList(*m_pRendererList);

		m_pRenderer = std::make_unique<Renderer>(m_pWindow->Gfx(), m_pLightManager, m_pRendererList);

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

	int App::Run()
	{
		m_pWindow->Show();

		while (true)
		{
			// process all messages pending, but to not block for new messages
			if (const auto ecode = DX11Window::ProcessMessages())
			{
				// if return optional has value, means we're quitting so return exit code
				return *ecode;
			}
		}
	}

	LRESULT App::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_PAINT:
			ExecuteFrame();
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void App::ExecuteFrame()
	{
		static float timeElapsed = 0.f;
		auto dt = m_pTimer->Mark();
		timeElapsed += dt;

		// Process input
		{
			auto& mouse = m_pWindow->GetMouse();
			while (!mouse.EventBufferIsEmpty())
			{
				MouseEvent evt = m_pWindow->GetMouse().ReadEvent();
				switch (evt.GetType())
				{
				case MouseEvent::EventType::LeftButtonDown:
				{
					m_pixelSelectionX = mouse.GetMousePosX();
					m_pixelSelectionY = mouse.GetMousePosY();
				}
				}
			}
		}

		// Update cameras
		{
			m_pCamera->Update();
		}

		// Animate lights
		{
			UINT ct = m_pLightManager->GetLightCount();
			for (UINT i = 0u; i < ct; ++i)
			{
				const auto light = m_pLightManager->GetLight(i);

				// Only move lights the user can't control
				if (!light->AllowUserControl())
				{
					auto positionWS = dx::XMLoadFloat3(&light->GetPositionWS());
					float theta = i * 0.52738f;
					float speed = 0.5f * dt;
					auto velWS = dx::XMVectorSet(std::sin(theta) * speed, 0.f, std::cos(theta) * speed, 0.f);
					positionWS = dx::XMVectorAdd(positionWS, velWS);

					light->SetPositionWS(positionWS);
				}
			}
		}

		m_pWindow->Gfx().BeginFrame();

		m_pRenderer->Execute(m_pWindow->Gfx(), m_pCamera, timeElapsed, m_pixelSelectionX, m_pixelSelectionY);

		// Draw Imgui windows
		{
			m_pRenderer->DrawImguiControlWindow(m_pWindow->Gfx());
			m_pCamera->DrawImguiControlWindow();
			m_pLightManager->DrawImguiControlWindows();
			m_pWindow->Gfx().GetLog()->DrawImguiControlWindow();
		}

		m_pWindow->Gfx().EndFrame();
		m_pRenderer->Reset();
	}
}