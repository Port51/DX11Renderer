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
	App();
	~App();
	// master frame / message loop
	int Go();
private:
	void DoFrame();
private:
	ImguiManager imgui; // must be initialized before wnd
	Window wnd;
	FrameCommander fc;
	Timer timer;
	bool showDemoWindow = true;
	Camera cam;
	PointLight light;
	std::unique_ptr<ModelInstance> model;
	float simulationSpeed = 1.f;
	std::vector<std::unique_ptr<MeshRenderer>> drawables;
	static constexpr size_t nDrawables = 1;
};