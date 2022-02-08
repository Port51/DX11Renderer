#pragma once
#include "Window.h"

namespace gfx
{
	class MeshRenderer;
	class RendererList;
	class ModelInstance;
	class LightManager;
	class Renderer;
	class ImguiManager;
	class Window;
	class Camera;
	class Timer;

	class App
	{
	public:
		App(int screenWidth, int screenHeight);
		virtual ~App();
		// master frame / message loop
		int Go();
	private:
		void DoFrame();
	private:
		std::unique_ptr<ImguiManager> pImgui; // must be initialized before wnd
		std::unique_ptr<Window> pWindow;
		std::unique_ptr<Camera> pCamera;
		std::shared_ptr<LightManager> pLightManager;
		std::unique_ptr<Renderer> pRenderer;
		std::unique_ptr<Timer> pTimer;

		std::unique_ptr<ModelInstance> pModel0;
		std::unique_ptr<ModelInstance> pModel1;
		std::vector<std::unique_ptr<MeshRenderer>> pDrawables;
		std::shared_ptr<RendererList> pRendererList;

		bool showDemoWindow = true;

		static constexpr UINT nDrawables = 1;
	};
}