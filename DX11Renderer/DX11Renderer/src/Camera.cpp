#include "pch.h"
#include "Camera.h"
#include "RenderConstants.h"
#include "Frustum.h"

namespace gfx
{
	Camera::Camera(float fov, float aspect, float nearClipPlane, float farClipPlane)
		: m_fov(fov), m_aspect(aspect), m_nearClipPlane(nearClipPlane), m_farClipPlane(farClipPlane)
	{
		UpdateProjectionMatrix();
	}

	const dx::XMVECTOR Camera::GetPositionWS() const
	{
		// LookAt fails if position = target
		const auto safeRad = dx::XMMax(m_radius, 0.01f);
		return dx::XMVectorSetW(dx::XMVectorScale(GetForwardWS(), -safeRad), 1.f);
	}

	const dx::XMVECTOR Camera::GetForwardWS() const
	{
		return dx::XMVectorSetW(dx::XMVector3Transform(
			dx::XMVectorSet(0.f, 0.f, 1.f, 0.f),
			dx::XMMatrixRotationRollPitchYaw(m_phi, -m_theta, 0.0f) // rotate that offset
		), 0.f);
	}

	const dx::XMMATRIX Camera::GetViewMatrix() const
	{
		// Apply look-at and local orientation
		// +Y = up
		return dx::XMMatrixLookAtLH(GetPositionWS(), dx::XMVectorZero(), dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
			* dx::XMMatrixRotationRollPitchYaw(m_pitch, -m_yaw, m_roll);
	}

	const dx::XMMATRIX Camera::GetProjectionMatrix() const
	{
		return m_projectionMatrix;
	}

	void Camera::SetFOV(float _fov)
	{
		m_fov = _fov;
		UpdateProjectionMatrix();
	}

	void Camera::SetAspect(float _aspect)
	{
		m_aspect = _aspect;
		UpdateProjectionMatrix();
	}

	/// Returns frustum corners in VS
	/// XY = distance 1, ZW = far
	/// Use in shader with NDC to get view dir or reconstruct positionVS
	/// This assumes a "normal" camera frustum, so don't try this with a planar reflection camera!
	const dx::XMVECTOR Camera::GetFrustumCornersVS() const
	{
		// Reference: http://davidlively.com/programming/graphics/frustum-calculation-and-culling-hopefully-demystified/

		// FOV = entire frustum angle
		float vFov = dx::XMConvertToRadians(m_fov);
		float halfAngleY = std::tan(vFov * 0.5f);
		float halfAngleX = halfAngleY * m_aspect;

		// Flip Y
		return dx::XMVectorSet(halfAngleX, -halfAngleY, halfAngleX * m_farClipPlane, -halfAngleY * m_farClipPlane);
	}

	const Frustum Camera::GetFrustumWS() const
	{
		Frustum frustum;
		return frustum;
	}

	const float Camera::GetNearClipPlane() const
	{
		return m_nearClipPlane;
	}

	const float Camera::GetFarClipPlane() const
	{
		return m_farClipPlane;
	}

	void Camera::UpdateProjectionMatrix()
	{
		m_projectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(m_fov), m_aspect, m_nearClipPlane, m_farClipPlane);
	}

	void Camera::DrawImguiControlWindow()
	{
		if (ImGui::Begin("Camera"))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("R", &m_radius, 0.0f, 80.0f, "%.1f");
			ImGui::SliderAngle("Theta", &m_theta, -180.0f, 180.0f);
			ImGui::SliderAngle("Phi", &m_phi, -89.0f, 89.0f);
			ImGui::Text("Orientation");
			ImGui::SliderAngle("Roll", &m_roll, -180.0f, 180.0f);
			ImGui::SliderAngle("Pitch", &m_pitch, -180.0f, 180.0f);
			ImGui::SliderAngle("Yaw", &m_yaw, -180.0f, 180.0f);

			ImGui::Text("Settings");
			float newFov = m_fov;
			ImGui::SliderFloat("FOV", &newFov, 10.0f, 180.0f, "%.1f");
			if (std::abs(newFov - m_fov) > FLT_EPSILON)
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
		m_radius = 20.0f;
		m_theta = 0.0f;
		m_phi = 0.0f;
		m_pitch = 0.0f;
		m_yaw = 0.0f;
		m_roll = 0.0f;
		m_fov = 40.0f;
	}
}