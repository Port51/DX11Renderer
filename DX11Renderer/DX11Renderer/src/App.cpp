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
		static u32 frameCt = 0u;
		frameCt++;

		static float timeElapsed = 0.f;
		const float dt = m_pTimer->Mark();
		const float limitedDt = std::min(dt, 1.f / 30.f);
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
			m_pCamera->Update(frameCt);
		}

		// Animate boats
		// todo: clean this up and move it to another class!!
		{
			const size_t ct = (instanceBoats) ? m_boatTransforms.size() : m_pBoatModels.size();
			for (size_t i = 0; i < ct; ++i)
			{
				auto centerOffset = GerstnerWaves::GetGerstnerWaveOffset(dx::XMLoadFloat3(&m_boatTransforms[i].position), timeElapsed);
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
					const float theta = static_cast<float>(si) * dx::XM_2PI / 8.f + m_boatTransforms[i].rotation.y;
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
				const float rotationScale = 5.51f * std::clamp(timeElapsed / 8.0f, 0.f, 1.f);
				auto angularAccel = dx::XMVectorSet(pitchOffset * -0.225f * rotationScale, 0.f, rollOffset * 1.2f * rotationScale, 0.f);

				// Add spring force that "rights" the boat
				const float uprightForce = 3.75f;
				angularAccel = dx::XMVectorSubtract(angularAccel, dx::XMVectorScale(dx::XMLoadFloat3(&m_boatTransforms[i].rotation), uprightForce));

				// Add damping force that opposes velocity
				const float dampingForce = 0.225f;
				angularAccel = dx::XMVectorSubtract(angularAccel, dx::XMVectorScale(m_boatAngularVelocities[i], dampingForce));

				// Lock yaw velocity to 0
				m_boatAngularVelocities[i] = dx::XMVectorSetY(dx::XMVectorAdd(m_boatAngularVelocities[i], dx::XMVectorScale(angularAccel, limitedDt)), 0.0f);
				auto angularPos = dx::XMLoadFloat3(&m_boatTransforms[i].rotation);
				angularPos = dx::XMVectorAdd(angularPos, dx::XMVectorScale(m_boatAngularVelocities[i], limitedDt));

				dx::XMStoreFloat3(&m_boatTransforms[i].position, buoyancyCenter);
				dx::XMStoreFloat3(&m_boatTransforms[i].rotation, angularPos);

				if (instanceBoats)
				{
					auto& boats = static_cast<InstancedModel&>(*m_pBoatModels.at(0));
					auto data = boats.GetInstanceDataPoint(i);
					auto& initialTransform = m_boatTransforms.at(i);
					
					auto newTransform = dx::XMMatrixScalingFromVector(dx::XMLoadFloat3(&initialTransform.scale))
						* dx::XMMatrixRotationRollPitchYawFromVector(angularPos)
						* dx::XMMatrixTranslationFromVector(buoyancyCenter);

					//newTransform = dx::XMMatrixMultiply(newTransform, m_sceneTransform);
					dx::XMStoreFloat4x4(&data.transform, newTransform);

					boats.SetInstanceDataPoint(i, data);
				}
				else
				{
					m_pBoatModels[i]->SetPositionWS(buoyancyCenter, false);
					m_pBoatModels[i]->SetRotationWS(angularPos, false);
					m_pBoatModels[i]->UpdateTransform();
				}
			}

			if (instanceBoats)
			{
				auto& boats = static_cast<InstancedModel&>(*m_pBoatModels.at(0));
				boats.ApplyInstanceData(Gfx());
			}
		}

		// Animate gems
		{
			if (instanceGems)
			{
				auto& gems = static_cast<InstancedModel&>(*m_pGemModels.at(0));
				for (size_t i = 0u; i < gems.GetInstanceCount(); ++i)
				{
					auto data = gems.GetInstanceDataPoint(i);
					auto& initialTransform = m_gemStartTransforms.at(i);

					// Rotate by random vector!
					auto rotation = dx::XMVector3Normalize(dx::XMVectorSet(std::sin(i * 12.389213), std::cos(i * 14.389213), std::sin(i * 21.889213), 0.f));
					rotation = dx::XMVectorScale(rotation, 0.7f * timeElapsed);

					auto newTransform = dx::XMMatrixScalingFromVector(dx::XMLoadFloat3(&initialTransform.scale))
						* dx::XMMatrixRotationRollPitchYawFromVector(rotation)
						* dx::XMMatrixTranslationFromVector(dx::XMLoadFloat3(&initialTransform.position));

					newTransform = dx::XMMatrixMultiply(newTransform, m_sceneTransform);
					dx::XMStoreFloat4x4(&data.transform, newTransform);
					
					gems.SetInstanceDataPoint(i, data);
				}
				gems.ApplyInstanceData(Gfx());
			}
			else
			{
				for (size_t i = 0; i < m_pGemModels.size(); ++i)
				{
					m_pGemModels[i]->SetYaw(m_pGemModels[i]->GetYaw() + limitedDt * 0.5f);
				}
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

		m_pRenderer->Execute(Gfx(), *m_pCamera.get(), frameCt, dt, timeElapsed, m_pixelSelectionX, m_pixelSelectionY);

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
		m_sceneTransform = dx::XMMatrixTranslation(2.f, -4.f, 0.f);

		// Moonlight
		m_pLightManager->AddDirectionalLight(Gfx(), 30.f, 30.f, dx::XMFLOAT3(1.f, 1.f, 1.f), 0.25f);

		// Add castle
		{
			const auto pCastleAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\NewCastle.asset");
			m_pCastleModel = std::make_unique<Model>(Gfx(), pCastleAsset, m_sceneTransform);
			m_pRendererList->AddModel(*m_pCastleModel);
		}

		// Add static instanced objects
		{
			std::vector<const char*> assets {
				"Assets\\Models\\WindowA.asset",
				"Assets\\Models\\GLTF\\NewCastle_WindowAPlacements.glb",
				"Assets\\Models\\RampartA.asset",
				"Assets\\Models\\GLTF\\NewCastle_RampartAPlacements.glb",
				"Assets\\Models\\TowerRampartA.asset",
				"Assets\\Models\\GLTF\\NewCastle_TowerRampartAPlacements.glb"
			};

			for (size_t i = 0; i < assets.size() / 2u; ++i)
			{
				const auto pAsset = ModelImporter::LoadGLTF(Gfx(), assets.at(i * 2u + 0u));
				auto placements = ModelImporter::LoadGLTFTransforms(Gfx(), assets.at(i * 2u + 1u));
				for (int i = 0, ct = placements.size(); i < ct; ++i)
				{
					dx::XMStoreFloat4x4(&placements[i].trs, dx::XMMatrixMultiply(dx::XMLoadFloat4x4(&placements[i].trs), m_sceneTransform));
				}

				m_pStaticInstancedModels.emplace_back(std::make_unique<InstancedModel>(Gfx(), pAsset, dx::XMMatrixIdentity(), placements, *m_pRandomGenerator));
				m_pRendererList->AddModel(*m_pStaticInstancedModels.at(m_pStaticInstancedModels.size() - 1u));
			}
		}

		// Add magic lights (static)
		{
			const auto pGemAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Gem.asset");
			auto gemPlacements = ModelImporter::LoadGLTFTransforms(Gfx(), "Assets\\Models\\GLTF\\NewCastle_TorchPlacements.glb");
			m_gemStartTransforms.reserve(gemPlacements.size());
			for (const auto& p : gemPlacements)
			{
				m_gemStartTransforms.emplace_back(p);

				const auto transformedPos = dx::XMLoadFloat4x4(&p.trs) * m_sceneTransform;
				const auto lightPosition = dx::XMVector3Transform(dx::XMLoadFloat3(&p.position), m_sceneTransform);
				m_pLightManager->AddPointLight(Gfx(), lightPosition, GetRandomMagicLight(), 2.8f * std::sqrt(p.scale.x), 1.f, 3.15f * p.scale.x);

				if (!instanceGems)
				{
					m_pGemModels.emplace_back(std::make_unique<Model>(Gfx(), pGemAsset, transformedPos));
					m_pRendererList->AddModel(*m_pGemModels.at(m_pGemModels.size() - 1u));
				}
				// Note - dx::XMFLOAT3(1.f, 0.25f, 0.04f) was a good color for torches
			}

			if (instanceGems)
			{
				// Apply scene transform to all instances
				for (int i = 0, ct = gemPlacements.size(); i < ct; ++i)
				{
					dx::XMStoreFloat4x4(&gemPlacements[i].trs, dx::XMMatrixMultiply(dx::XMLoadFloat4x4(&gemPlacements[i].trs), m_sceneTransform));
				}

				// Setup instances
				m_pGemModels.emplace_back(std::make_unique<InstancedModel>(Gfx(), pGemAsset, dx::XMMatrixIdentity(), gemPlacements, *m_pRandomGenerator));
				m_pRendererList->AddModel(*m_pGemModels.at(m_pGemModels.size() - 1u));
			}
		}

		// Add people (static)
		{
			const auto pPersonAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Person.asset");
			auto personPlacements = ModelImporter::LoadGLTFTransforms(Gfx(), "Assets\\Models\\GLTF\\NewCastle_PersonPlacements.glb");
			for (const auto& p : personPlacements)
			{
				const auto transformedPos = dx::XMLoadFloat4x4(&p.trs) * m_sceneTransform;
				
				// Mask out translation by setting W = 0
				const auto lightOffset = dx::XMVector4Transform(dx::XMVectorSet(0.2f, 1.41f, -0.28f, 0.f), dx::XMMatrixRotationRollPitchYawFromVector(dx::XMLoadFloat3(&p.rotation)));
				const auto lightPosition = dx::XMVector3Transform(dx::XMVectorAdd(dx::XMLoadFloat3(&p.position), lightOffset), m_sceneTransform);
				m_pLightManager->AddPointLight(Gfx(), lightPosition, dx::XMFLOAT3(0.f / 255.f, 98.f / 255.f, 255.f / 255.f), 2.8f, 1.f, 3.15f);

				if (!instancePeople)
				{
					m_pPersonModels.emplace_back(std::make_unique<Model>(Gfx(), pPersonAsset, transformedPos));
					m_pRendererList->AddModel(*m_pPersonModels.at(m_pPersonModels.size() - 1u));
				}
			}

			if (instancePeople)
			{
				// Apply scene transform to all instances
				for (int i = 0, ct = personPlacements.size(); i < ct; ++i)
				{
					dx::XMStoreFloat4x4(&personPlacements[i].trs, dx::XMMatrixMultiply(dx::XMLoadFloat4x4(&personPlacements[i].trs), m_sceneTransform));
				}

				// Setup instances
				m_pPersonModels.emplace_back(std::make_unique<InstancedModel>(Gfx(), pPersonAsset, dx::XMMatrixIdentity(), personPlacements, *m_pRandomGenerator));
				m_pRendererList->AddModel(*m_pPersonModels.at(m_pPersonModels.size() - 1u));
			}
		}

		// Add boat placements
		{
			const auto pBoatAsset = ModelImporter::LoadGLTF(Gfx(), "Assets\\Models\\Boat.asset");
			auto boatPlacements = ModelImporter::LoadGLTFTransforms(Gfx(), "Assets\\Models\\GLTF\\NewCastle_BoatPlacements.glb");

			m_boatTransforms.reserve(boatPlacements.size());
			m_boatStartPositions.reserve(boatPlacements.size());
			m_boatVelocities.resize(boatPlacements.size());
			m_boatAngularVelocities.resize(boatPlacements.size());

			if (instanceBoats)
			{
				// Apply scene transform to all instances
				for (auto& p : boatPlacements)
				{
					m_boatTransforms.emplace_back(p);
					dx::XMStoreFloat4x4(&p.trs, dx::XMMatrixMultiply(dx::XMLoadFloat4x4(&p.trs), m_sceneTransform));

					const auto transformedPos = dx::XMVector3Transform(dx::XMLoadFloat3(&p.position), m_sceneTransform);
					m_boatStartPositions.emplace_back(transformedPos);
				}

				// Setup instances
				m_pBoatModels.emplace_back(std::make_unique<InstancedModel>(Gfx(), pBoatAsset, dx::XMMatrixIdentity(), boatPlacements, *m_pRandomGenerator));
				m_pRendererList->AddModel(*m_pBoatModels.at(m_pBoatModels.size() - 1u));
			}
			else
			{
				for (auto& p : boatPlacements)
				{
					m_pBoatModels.emplace_back(std::make_unique<Model>(Gfx(), pBoatAsset, dx::XMLoadFloat4x4(&p.trs) * m_sceneTransform));
					m_pRendererList->AddModel(*m_pBoatModels.at(m_pBoatModels.size() - 1u));

					m_boatStartPositions.emplace_back(m_pBoatModels.at(m_pBoatModels.size() - 1u)->GetPositionWS());
				}
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