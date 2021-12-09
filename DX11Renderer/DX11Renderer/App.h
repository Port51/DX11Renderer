#pragma once
#include "Window.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "ModelInstance.h"
#include "FrameCommander.h"

class MeshRenderer;

class App
{
public:
	App(int screenWidth, int screenHeight);
	~App();
	// master frame / message loop
	int Go();
private:
	void DoFrame();
private:
	ImguiManager imgui; // must be initialized before wnd
	Window wnd;
	Timer timer;
	bool showDemoWindow = true;
	Camera cam;
	std::unique_ptr<ModelInstance> pModel0;
	std::unique_ptr<ModelInstance> pModel1;
	float simulationSpeed = 1.f;
	std::unique_ptr<FrameCommander> fc;

	std::vector<std::unique_ptr<MeshRenderer>> pDrawables;
	std::vector<std::shared_ptr<PointLight>> pLights;
	static constexpr size_t nDrawables = 1;
};