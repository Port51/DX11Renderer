#pragma once
#include "Window.h"
#include "Timer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"

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
	Timer timer;
	bool showDemoWindow = true;
	Camera cam;
	PointLight light;
	float simulationSpeed = 1.f;
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 180;
};