#include "Camera.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

Camera::Camera(float fov, float aspect)
	: fov(fov), aspect(aspect)
{
	UpdateProjectionMatrix();
}

dx::XMMATRIX Camera::GetViewMatrix() const
{
	// LookAt fails if position = target
	const auto safeRad = dx::XMMax(r, 0.01f);
	const auto pos = dx::XMVector3Transform(
		dx::XMVectorSet(0.0f, 0.0f, -safeRad, 0.0f), // move camera back in Z axis
		dx::XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f) // rotate that offset
	);

	// Apply look-at and local orientation
	// +Y = up
	return dx::XMMatrixLookAtLH(pos, dx::XMVectorZero(), dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
		* dx::XMMatrixRotationRollPitchYaw(pitch, -yaw, roll);
}

dx::XMMATRIX Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

void Camera::SetFOV(float _fov)
{
	fov = _fov;
	UpdateProjectionMatrix();
}

void Camera::SetAspect(float _aspect)
{
	aspect = _aspect;
	UpdateProjectionMatrix();
}

void Camera::UpdateProjectionMatrix()
{
	projectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(fov), aspect, 0.5f, 100.0f);
}

void Camera::DrawImguiControlWindow()
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("R", &r, 0.0f, 80.0f, "%.1f");
		ImGui::SliderAngle("Theta", &theta, -180.0f, 180.0f);
		ImGui::SliderAngle("Phi", &phi, -89.0f, 89.0f);
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}

void Camera::Reset()
{
	r = 20.0f;
	theta = 0.0f;
	phi = 0.0f;
	pitch = 0.0f;
	yaw = 0.0f;
	roll = 0.0f;
}