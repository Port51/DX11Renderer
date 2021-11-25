#pragma once
#include "Graphics.h"

class Camera
{
public:
	Camera(float fov, float aspect);
public:
	dx::XMMATRIX GetViewMatrix() const;
	dx::XMMATRIX GetProjectionMatrix() const;
	void SetFOV(float fov);
	void SetAspect(float aspect);
	void DrawImguiControlWindow();
	void Reset();
private:
	void UpdateProjectionMatrix();
private:
	float r = 13.3f; // dist from origin
	float theta = -0.471225f; // rotate around origin
	float phi = 0.209f; // rotate around origin
	float pitch = 0.061f;
	float yaw = 0.0f;
	float roll = 0.0f;
	float fov = 40.0f;
	float aspect = 0.75f;
	dx::XMMATRIX projectionMatrix;
};