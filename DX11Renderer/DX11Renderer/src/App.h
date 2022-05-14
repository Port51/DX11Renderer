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
	class ModelInstance;
	class LightManager;
	class Renderer;
	class ImguiManager;
	class DX11Window;
	class Camera;

	class App : public InputListener
	{
	public:
		App(UINT screenWidth, UINT screenHeight, HINSTANCE hInstance);
		virtual ~App();
		// master frame / message loop
		int Run();

		LRESULT CALLBACK HandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
		GraphicsDevice& Gfx() const;
	private:
		void ExecuteFrame();
	private:
		std::unique_ptr<ImguiManager> m_pImgui; // must be initialized before wnd
		std::unique_ptr<DX11Window> m_pWindow;
		std::unique_ptr<GraphicsDevice> m_pGfx;
		std::unique_ptr<Camera> m_pCamera;
		std::shared_ptr<LightManager> m_pLightManager;
		std::unique_ptr<Renderer> m_pRenderer;
		std::unique_ptr<Timer> m_pTimer;

		std::unique_ptr<ModelInstance> m_pModel0;
		std::unique_ptr<ModelInstance> m_pModel1;
		std::vector<std::unique_ptr<MeshRenderer>> m_pDrawables;
		std::shared_ptr<RendererList> m_pRendererList;

		bool m_showDemoWindow = true;
		UINT m_pixelSelectionX = 450u;
		UINT m_pixelSelectionY = 480u;
		UINT m_pixelIteration = 0u;

		static constexpr UINT m_nDrawables = 1;
	};
}