#pragma once
#include "Graphics.h"

class Camera
{
public:
	DirectX::XMMATRIX GetViewMatrix() const;
	void DrawImguiControlWindow();
	void Reset();
private:
	float r = 14.2f; // dist from origin
	float theta = -2.69f; // rotate around origin
	float phi = 0.209f; // rotate around origin
	float pitch = 0.061f;
	float yaw = 0.0f;
	float roll = 0.0f;
};