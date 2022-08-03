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
#include "RenderStats.h"
#include "RandomGenerator.h"
#include "ParticleManager.h"

namespace gfx
{
	App::App(const UINT screenWidth, const UINT screenHeight, HINSTANCE hInstance)
		:
		m_pImgui(std::make_unique<ImguiManager>()),
		m_pCamera(std::make_unique<Camera>(40.0f, (float)screenWidth / (float)screenHeight, 0.5f, 1000.0f)),
		m_pTimer(std::make_unique<Timer>()),
		m_pRendererList(std::make_shared<RendererList>()),
		m_pRandomGenerator(std::make_unique<RandomGenerator>())
	{
		m_pWindow = std::make_unique<DX11Window>(screenWidth, screenHeight, "DX11 Renderer", hInstance, this);

		// Create graphics
		m_pGfx = std::make_unique<GraphicsDevice>(m_pWindow->GetHwnd(), screenWidth, screenHeight);

		m_pLightManager = std::make_shared<LightManager>(Gfx(), m_pRendererList);
		m_pParticleManager = std::make_shared<ParticleManager>(Gfx());

		CreateCastleScene();

		m_pLightManager->AddLightModelsToList(*m_pRendererList);
		m_pRenderer = std::make_unique<Renderer>(Gfx(), *m_pRandomGenerator, m_pLightManager, m_pParticleManager, m_pRendererList);
	}

	App::~App()
	{
		m_pRenderer->Release();
	}

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

	LRESULT App::HandleMsg(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
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

	GraphicsDevice& App::Gfx() const
	{
		return *m_pGfx;
	}

	void App::ExecuteFrame()
	{
		static float timeElapsed = 0.f;
		const float dt = m_pTimer->Mark();
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
		if (false)
		{
			UINT ct = m_pLightManager->GetLightCount();
			for (UINT i = 0u; i < ct; ++i)
			{
				auto& light = m_pLightManager->GetLight(i);

				// Only move lights the user can't control
				if (!light.AllowUserControl())
				{
					dx::XMFLOAT3 positionWS_f3 = light.GetPositionWS();
					dx::XMVECTOR positionWS = dx::XMLoadFloat3(&positionWS_f3);
					const float theta = i * 0.52738f;
					const float speed = 0.5f * dt;
					const auto velWS = dx::XMVectorSet(std::sin(theta) * speed, 0.f, std::cos(theta) * speed, 0.f);
					positionWS = dx::XMVectorAdd(positionWS, velWS);

					light.SetPositionWS(positionWS);
				}
			}
		}

		Gfx().BeginFrame();

		m_pRenderer->Execute(Gfx(), *m_pCamera.get(), dt, timeElapsed, m_pixelSelectionX, m_pixelSelectionY);

		// Draw Imgui windows
		{
			m_pRenderer->DrawImguiControlWindow(Gfx());
			m_pCamera->DrawImguiControlWindow();
			m_pLightManager->DrawImguiControlWindows();
			Gfx().GetLog().DrawImguiControlWindow();
			Gfx().GetRenderStats().DrawImguiControlWindow();
		}

		Gfx().EndFrame();
		m_pRenderer->Reset();
	}

	void App::CreateCastleScene()
	{
		auto sceneTransform = dx::XMMatrixTranslation(2.f, -5.f, 0.f);

		// Moonlight
		m_pLightManager->AddDirectionalLight(Gfx(), 30.f, 30.f, dx::XMFLOAT3(1.f, 1.f, 1.f), 0.25f);

		// Add castle
		{
			auto pCastleAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\NewCastle.asset");
			m_pCastleModel = std::make_unique<ModelInstance>(Gfx(), pCastleAsset, sceneTransform);
			m_pRendererList->AddModelInstance(*m_pCastleModel);
		}

		// Add torches
		{
			const auto torchPlacements = ModelImporter::LoadGLTFPositions(Gfx(), "Assets\\Models\\GLTF\\NewCastle_TorchPlacements.glb");
			for (const auto& tp : torchPlacements)
			{
				auto transformedPos = dx::XMVector4Transform(dx::XMVectorSet(tp.x, tp.y, tp.z, 1.0), sceneTransform);
				dx::XMFLOAT3 f3;
				dx::XMStoreFloat3(&f3, transformedPos);
				m_pLightManager->AddPointLight(Gfx(), f3, dx::XMFLOAT3(1.f, 0.25f, 0.04f), 1.8f, 1.f, 2.15f);
			}
		}

		// Add boat placements
		{
			auto pBoatAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Boat.asset");
			auto boatPlacements = ModelImporter::LoadGLTFTransforms(Gfx(), "Assets\\Models\\GLTF\\NewCastle_BoatPlacements.glb");
			for (const auto& bp : boatPlacements)
			{
				m_pBoatModels.emplace_back(std::make_unique<ModelInstance>(Gfx(), pBoatAsset, dx::XMLoadFloat4x4(&bp) * sceneTransform));
				m_pRendererList->AddModelInstance(*m_pBoatModels.at(m_pBoatModels.size() - 1u));
			}
		}

		// Add lanterns
		auto pLanternAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\FloatingLantern.asset");
		{
			
		}
	}
}