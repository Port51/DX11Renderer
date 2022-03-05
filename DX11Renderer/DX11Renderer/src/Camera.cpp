#include "pch.h"
#include "Camera.h"
#include "RenderConstants.h"
#include "Frustum.h"

namespace gfx
{
	Camera::Camera(float fov, float aspect, float nearClipPlane, float farClipPlane)
		: fov(fov), aspect(aspect), nearClipPlane(nearClipPlane), farClipPlane(farClipPlane)
	{
		UpdateProjectionMatrix();
	}

	dx::XMVECTOR Camera::GetPositionWS() const
	{
		// LookAt fails if position = target
		const auto safeRad = dx::XMMax(r, 0.01f);
		return dx::XMVectorSetW(dx::XMVectorScale(GetForwardWS(), -safeRad), 1.f);
	}

	dx::XMVECTOR Camera::GetForwardWS() const
	{
		return dx::XMVectorSetW(dx::XMVector3Transform(
			dx::XMVectorSet(0.f, 0.f, 1.f, 0.f),
			dx::XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f) // rotate that offset
		), 0.f);
	}

	dx::XMMATRIX Camera::GetViewMatrix() const
	{
		// Apply look-at and local orientation
		// +Y = up
		return dx::XMMatrixLookAtLH(GetPositionWS(), dx::XMVectorZero(), dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
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

	/// Returns frustum corners in VS
	/// XY = distance 1, ZW = far
	/// Use in shader with NDC to get view dir or reconstruct positionVS
	/// This assumes a "normal" camera frustum, so don't try this with a planar reflection camera!
	dx::XMVECTOR Camera::GetFrustumCornersVS() const
	{
		// Reference: http://davidlively.com/programming/graphics/frustum-calculation-and-culling-hopefully-demystified/

		// FOV = entire frustum angle
		float vFov = dx::XMConvertToRadians(fov);
		float halfAngleY = std::tan(vFov * 0.5f);
		float halfAngleX = halfAngleY * aspect;

		// Flip Y
		return dx::XMVectorSet(halfAngleX, -halfAngleY, halfAngleX * farClipPlane, -halfAngleY * farClipPlane);
	}

	Frustum Camera::GetFrustumWS() const
	{
		Frustum frustum;
		return frustum;
	}

	float Camera::GetNearClipPlane() const
	{
		return nearClipPlane;
	}

	float Camera::GetFarClipPlane() const
	{
		return farClipPlane;
	}

	void Camera::UpdateProjectionMatrix()
	{
		projectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(fov), aspect, nearClipPlane, farClipPlane);
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

			ImGui::Text("Settings");
			float newFov = fov;
			ImGui::SliderFloat("FOV", &newFov, 10.0f, 180.0f, "%.1f");
			if (std::abs(newFov - fov) > FLT_EPSILON)
			{
				SetFOV(newFov);
			}

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
		fov = 40.0f;
	}
}