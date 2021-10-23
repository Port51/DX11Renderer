#pragma once
#include "Graphics.h"

class Camera
{
public:
	DirectX::XMMATRIX GetViewMatrix() const noexcept;
	void DrawImguiControlWindow() noexcept;
	void Reset() noexcept;
private:
	float r = 20.0f; // dist from origin
	float theta = 0.0f; // rotate around origin
	float phi = 0.0f; // rotate around origin
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
};