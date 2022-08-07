#include "pch.h"
#include "App.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "RendererList.h"
#include "BaseModel.h"
#include "Model.h"
#include "InstancedModel.h"
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
#include "GerstnerWaves.h"

namespace gfx
{
	App::App(const UINT screenWidth, const UINT screenHeight, HINSTANCE hInstance)
		:
		m_pImgui(std::make_unique<ImguiManager>()),
		m_pCamera(std::make_unique<Camera>(55.0f, (float)screenWidth / (float)screenHeight, 0.5f, 1000.0f)),
		m_pTimer(std::make_unique<Timer>()),
		m_pRendererList(std::make_shared<RendererList>()),
		m_pRandomGenerator(std::make_unique<RandomGenerator>())
	{
		m_pWindow = std::make_unique<DX11Window>(screenWidth, screenHeight, "DX11 Renderer", hInstance, this);

		// Create graphics
		m_pGfx = std::make_unique<GraphicsDevice>(m_pWindow->GetHwnd(), screenWidth, screenHeight);

		m_pLightManager = std::make_shared<LightManager>(Gfx(), m_pRendererList);
		//m_pParticleManager = std::make_shared<ParticleManager>(Gfx());

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

		// Animate boats
		{
			for (size_t i = 0; i < m_pBoatModels.size(); ++i)
			{
				auto centerOffset = GerstnerWaves::GetGerstnerWaveOffset(m_pBoatModels.at(i)->GetPositionWS(), timeElapsed);
				// Dampen motion
				centerOffset = dx::XMVectorMultiply(centerOffset, dx::XMVectorSet(0.5f, 0.9f, 0.5f, 1.f));
				auto buoyancyCenter = dx::XMVectorAdd(m_boatStartPositions.at(i), centerOffset);

				// Adjust to Y value
				buoyancyCenter = dx::XMVectorSetY(buoyancyCenter, GerstnerWaves::GetGerstnerWaveVerticalDisplacementEstimate(buoyancyCenter, timeElapsed, 4u));

				float pitchOffset = 0.f;
				float rollOffset = 0.f;
				float sumOffsetY = dx::XMVectorGetY(buoyancyCenter) * 2.f;

				// Estimate rotation via samples in oval pattern
				for (size_t si = 0; si < 8; ++si)
				{
					const float theta = static_cast<float>(si) * dx::XM_2PI / 8.f + m_pBoatModels.at(i)->GetYaw();
					const float s = std::sin(theta) * 2.f; // oval shape
					const float c = std::cos(theta);
					const float dist = 0.2f;
					const auto sampleOffset = dx::XMVectorSet(c * dist, 0.f, s * dist, 0.f);
					const auto samplePos = dx::XMVectorAdd(sampleOffset, buoyancyCenter);

					const auto sampleY = GerstnerWaves::GetGerstnerWaveVerticalDisplacementEstimate(samplePos, timeElapsed, 4u);

					pitchOffset += sampleY * s;
					rollOffset += sampleY * c;
					sumOffsetY += sampleY;
				}

				// Set Y to average, with center weighted x2
				buoyancyCenter = dx::XMVectorSetY(buoyancyCenter, dx::XMVectorGetY(m_boatStartPositions.at(i)) + sumOffsetY / 10.f);

				//m_boatVelocities[i] = dx::XMVectorAdd(m_boatVelocities[i], dx::XMVectorScale(dx::XMVectorSubtract(buoyancyCenter, m_pBoatModels[i]->GetPositionWS()), dt));

				// Scale up over time to avoid spinning boats at the beginning
				const float rotationScale = 3.0f * std::clamp(timeElapsed / 8.0f, 0.f, 1.f);
				auto angularAccel = dx::XMVectorSet(pitchOffset * -0.225f * rotationScale, 0.f, rollOffset * 1.2f * rotationScale, 0.f);

				// Add spring force that "rights" the boat
				const float uprightForce = 4.0f;
				angularAccel = dx::XMVectorSubtract(angularAccel, dx::XMVectorScale(m_pBoatModels[i]->GetRotationWS(), uprightForce));

				// Add damping force that opposes velocity
				const float dampingForce = 0.25f;
				angularAccel = dx::XMVectorSubtract(angularAccel, dx::XMVectorScale(m_boatAngularVelocities[i], dampingForce));

				// Lock yaw velocity to 0
				m_boatAngularVelocities[i] = dx::XMVectorSetY(dx::XMVectorAdd(m_boatAngularVelocities[i], dx::XMVectorScale(angularAccel, dt)), 0.0f);
				auto angularPos = m_pBoatModels[i]->GetRotationWS();
				angularPos = dx::XMVectorAdd(angularPos, dx::XMVectorScale(m_boatAngularVelocities[i], dt));

				m_pBoatModels[i]->SetPositionWS(buoyancyCenter, false);
				m_pBoatModels[i]->SetRotationWS(angularPos, false);
				m_pBoatModels[i]->UpdateTransform();
			}
		}

		// Animate gems
		{
			for (size_t i = 0; i < m_pGemModels.size(); ++i)
			{
				m_pGemModels[i]->SetYaw(m_pGemModels[i]->GetYaw() + dt * 0.5f);
			}
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
					const float theta = i * 0.52738f;
					const float speed = 0.5f * dt;
					const auto velWS = dx::XMVectorSet(std::sin(theta) * speed, 0.f, std::cos(theta) * speed, 0.f);
					light.SetPositionWS(dx::XMVectorAdd(light.GetPositionWS(), velWS));
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
		const auto sceneTransform = dx::XMMatrixTranslation(2.f, -5.f, 0.f);

		// Moonlight
		m_pLightManager->AddDirectionalLight(Gfx(), 30.f, 30.f, dx::XMFLOAT3(1.f, 1.f, 1.f), 0.25f);

		// Add castle
		{
			const auto pCastleAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\NewCastle.asset");
			m_pCastleModel = std::make_unique<Model>(Gfx(), pCastleAsset, sceneTransform);
			m_pRendererList->AddModel(*m_pCastleModel);
		}

		// Add magic lights (static)
		{
			const auto pGemAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Gem.asset");
			const auto torchPlacements = ModelImporter::LoadGLTFPositionsAndScales(Gfx(), "Assets\\Models\\GLTF\\NewCastle_TorchPlacements.glb");
			for (const auto& tp : torchPlacements)
			{
				const auto transformedPos = dx::XMVector4Transform(dx::XMVectorSet(tp.x, tp.y, tp.z, 1.0), sceneTransform);
				m_pLightManager->AddPointLight(Gfx(), transformedPos, GetRandomMagicLight(), 2.8f * std::sqrt(tp.w), 1.f, 3.15f * tp.w);

				m_pGemModels.emplace_back(std::make_unique<Model>(Gfx(), pGemAsset, dx::XMMatrixScaling(tp.w, tp.w, tp.w) * dx::XMMatrixRotationRollPitchYaw(0.f, m_pRandomGenerator->GetUniformFloat(0.f, dx::XM_2PI), 0.f) * dx::XMMatrixTranslationFromVector(transformedPos)));
				m_pRendererList->AddModel(*m_pGemModels.at(m_pGemModels.size() - 1u));

				// dx::XMFLOAT3(1.f, 0.25f, 0.04f) was a good color for torches
			}
		}

		// Add people (static)
		{
			const bool instancePeople = true;

			const auto pPersonAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Person.asset");
			const auto personPlacements = ModelImporter::LoadGLTFTransforms(Gfx(), "Assets\\Models\\GLTF\\NewCastle_PersonPlacements.glb");
			for (const auto& tp : personPlacements)
			{
				const auto transformedPos = dx::XMLoadFloat4x4(&tp.trs) * sceneTransform;
				
				// Mask out translation by setting W = 0
				const auto lightOffset = dx::XMVector4Transform(dx::XMVectorSet(0.2f, 1.41f, -0.28f, 0.f), dx::XMMatrixRotationRollPitchYawFromVector(dx::XMLoadFloat3(&tp.rotation)));
				auto lightPosition = dx::XMVector3Transform(dx::XMVectorAdd(dx::XMLoadFloat3(&tp.position), lightOffset), sceneTransform);
				//lightPosition = dx::XMLoadFloat3(&tp.position);
				m_pLightManager->AddPointLight(Gfx(), lightPosition, dx::XMFLOAT3(0.f / 255.f, 98.f / 255.f, 255.f / 255.f), 2.8f, 1.f, 3.15f);

				if (!instancePeople)
				{
					m_pPersonModels.emplace_back(std::make_unique<Model>(Gfx(), pPersonAsset, transformedPos));
					m_pRendererList->AddModel(*m_pPersonModels.at(m_pPersonModels.size() - 1u));
				}
				// dx::XMFLOAT3(1.f, 0.25f, 0.04f) was a good color for torches
			}

			if (!instancePeople)
			{
				m_pPersonModels.emplace_back(std::make_unique<InstancedModel>(Gfx(), pPersonAsset, dx::XMMatrixIdentity()));
				m_pRendererList->AddModel(*m_pPersonModels.at(m_pPersonModels.size() - 1u));
			}
		}

		// Add boat placements
		{
			const auto pBoatAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Boat.asset");
			const auto boatPlacements = ModelImporter::LoadGLTFTransforms(Gfx(), "Assets\\Models\\GLTF\\NewCastle_BoatPlacements.glb");

			m_boatVelocities.resize(boatPlacements.size());
			m_boatAngularVelocities.resize(boatPlacements.size());
			for (const auto& bp : boatPlacements)
			{
				m_pBoatModels.emplace_back(std::make_unique<Model>(Gfx(), pBoatAsset, dx::XMLoadFloat4x4(&bp.trs) * sceneTransform));
				m_pRendererList->AddModel(*m_pBoatModels.at(m_pBoatModels.size() - 1u));

				m_boatStartPositions.emplace_back(m_pBoatModels.at(m_pBoatModels.size() - 1u)->GetPositionWS());
			}
		}

		// Add lanterns
		auto pLanternAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\FloatingLantern.asset");
		{
			
		}
	}

	const dx::XMFLOAT3 App::GetRandomMagicLight() const
	{
		float h = m_pRandomGenerator->GetUniformFloat01();
		float s = m_pRandomGenerator->GetUniformFloat01() * 0.4f + 0.6f;
		float v = 1.f;

		dx::XMFLOAT3 rgb;
		float C = v * s;
		float H = h * 6.f;
		float X = C * (1 - abs(fmod(H, 2) - 1));
		{
			int I = floor(H);
			if (I == 0) { rgb = dx::XMFLOAT3(C, X, 0.f); }
			else if (I == 1) { rgb = dx::XMFLOAT3(X, C, 0.f); }
			else if (I == 2) { rgb = dx::XMFLOAT3(0.f, C, X); }
			else if (I == 3) { rgb = dx::XMFLOAT3(0.f, X, C); }
			else if (I == 4) { rgb = dx::XMFLOAT3(X, 0.f, C); }
			else { rgb = dx::XMFLOAT3(C, 0, X); }
		}
		float M = v - C;
		return dx::XMFLOAT3(rgb.x + M, rgb.y + M, rgb.z + M);
	}
}