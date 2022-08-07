#pragma once
#include "InputListener.h"
#include "DX11Window.h"
#include "GraphicsDevice.h"

namespace gfxcore
{
	class Timer;
}

namespace gfx
{
	class MeshRenderer;
	class RendererList;
	class BaseModel;
	class LightManager;
	class Renderer;
	class ImguiManager;
	class DX11Window;
	class Camera;
	class RandomGenerator;
	class ParticleManager;

	class App : public InputListener
	{
	private:
		const bool instanceGems = true;
		const bool instancePeople = true;

	public:
		App(const UINT screenWidth, const UINT screenHeight, HINSTANCE hInstance);
		virtual ~App();
		// master frame / message loop
		int Run();

		LRESULT CALLBACK HandleMsg(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) override;
		GraphicsDevice& Gfx() const;
	private:
		void ExecuteFrame();
		void CreateCastleScene();
		const dx::XMFLOAT3 GetRandomMagicLight() const;
	private:
		std::unique_ptr<ImguiManager> m_pImgui; // must be initialized before wnd
		std::unique_ptr<DX11Window> m_pWindow;
		std::unique_ptr<GraphicsDevice> m_pGfx;
		std::unique_ptr<Camera> m_pCamera;
		std::shared_ptr<LightManager> m_pLightManager;
		std::shared_ptr<ParticleManager> m_pParticleManager;
		std::unique_ptr<Renderer> m_pRenderer;
		std::unique_ptr<Timer> m_pTimer;
		std::unique_ptr<RandomGenerator> m_pRandomGenerator;

		std::unique_ptr<BaseModel> m_pCastleModel;
		std::shared_ptr<RendererList> m_pRendererList;

		std::vector<std::unique_ptr<BaseModel>> m_pGemModels;
		std::vector<std::unique_ptr<BaseModel>> m_pBoatModels;
		std::vector<dx::XMVECTOR> m_boatStartPositions;
		std::vector<dx::XMVECTOR> m_boatVelocities;
		std::vector<dx::XMVECTOR> m_boatAngularVelocities;

		std::vector<std::unique_ptr<BaseModel>> m_pPersonModels;

		bool m_showDemoWindow = true;
		UINT m_pixelSelectionX = 450u;
		UINT m_pixelSelectionY = 480u;
		UINT m_pixelIteration = 0u;

		static constexpr UINT m_nDrawables = 1;
	};
}