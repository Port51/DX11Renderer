#pragma once
#include "InputListener.h"
#include "DX11Window.h"

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
	class Timer;

	class App : public InputListener
	{
	public:
		App(int screenWidth, int screenHeight, HINSTANCE hInstance);
		virtual ~App();
		// master frame / message loop
		int Run();

		LRESULT CALLBACK HandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	private:
		void DoFrame();
	private:
		std::unique_ptr<ImguiManager> pImgui; // must be initialized before wnd
		std::unique_ptr<DX11Window> pWindow;
		std::unique_ptr<Camera> pCamera;
		std::shared_ptr<LightManager> pLightManager;
		std::unique_ptr<Renderer> pRenderer;
		std::unique_ptr<Timer> pTimer;

		std::unique_ptr<ModelInstance> pModel0;
		std::unique_ptr<ModelInstance> pModel1;
		std::vector<std::unique_ptr<MeshRenderer>> pDrawables;
		std::shared_ptr<RendererList> pRendererList;

		bool showDemoWindow = true;
		UINT pixelSelectionX = 450u;
		UINT pixelSelectionY = 480u;
		UINT pixelIteration = 0u;

		static constexpr UINT nDrawables = 1;
	};
}