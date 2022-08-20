#include "pch.h"
#include "Camera.h"
#include "GraphicsDevice.h"
#include "RenderConstants.h"

namespace gfx
{
	Camera::Camera(const float fov, const float aspect, const float nearClipPlane, const float farClipPlane)
		: m_fov(fov), m_aspect(aspect), m_nearClipPlane(nearClipPlane), m_farClipPlane(farClipPlane)
	{
		UpdateProjectionMatrix();
		UpdateFrustumVS();
	}

	Camera::~Camera()
	{}

	const dx::XMMATRIX& Camera::GetViewMatrix() const
	{
		return m_viewMatrix;
	}

	const dx::XMMATRIX& Camera::GetInverseViewMatrix() const
	{
		return m_inverseViewMatrix;
	}

	const dx::XMMATRIX& Camera::GetViewProjectionMatrix() const
	{
		return m_viewProjectionMatrix;
	}

	const dx::XMMATRIX& Camera::GetInverseViewProjectionMatrix() const
	{
		return m_inverseViewProjectionMatrix;
	}

	const dx::XMMATRIX& Camera::GetProjectionMatrix() const
	{
		return m_projectionMatrix;
	}

	const dx::XMMATRIX& Camera::GetInverseProjectionMatrix() const
	{
		return m_inverseProjectionMatrix;
	}

	void Camera::SetFOV(const float _fov)
	{
		m_fov = _fov;
		UpdateProjectionMatrix();
		UpdateFrustumVS();
	}

	void Camera::SetAspect(const float _aspect)
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
		return dx::XMLoadFloat4(&m_frustumCornersVS);
	}

	const dx::XMVECTOR Camera::GetInverseFrustumCornersVS() const
	{
		return dx::XMLoadFloat4(&m_inverseFrustumCornersVS);
	}

	const dx::XMVECTOR Camera::GetGPUFrustumPlaneDirVS() const
	{
		return dx::XMLoadFloat4(&m_gpuFrustumPlaneDirVS);
	}

	const Frustum& Camera::GetFrustumWS() const
	{
		return m_frustumWS;
	}

	const Frustum& Camera::GetFrustumVS() const
	{
		return m_frustumVS;
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
		m_inverseProjectionMatrix = dx::XMMatrixInverse(nullptr, m_projectionMatrix);
	}

	void Camera::UpdateFrustumVS()
	{
		// Reference: http://davidlively.com/programming/graphics/frustum-calculation-and-culling-hopefully-demystified/

		// FOV = entire frustum angle
		float vFov = dx::XMConvertToRadians(m_fov);
		float halfAngleY = std::tan(vFov * 0.5f);
		float halfAngleX = halfAngleY * m_aspect;

		// Flip Y
		m_frustumCornersVS = dx::XMFLOAT4(halfAngleX, -halfAngleY, halfAngleX * m_farClipPlane, -halfAngleY * m_farClipPlane);
		m_inverseFrustumCornersVS = dx::XMFLOAT4(1.f / halfAngleX, -1.f / halfAngleY, 1.f / (halfAngleX * m_farClipPlane), -1.f / (halfAngleY * m_farClipPlane));

		m_frustumVS.UpdatePlanesFromMatrix(GetProjectionMatrix());
		//m_frustumVS.UpdatePlanesFromViewSpaceCorners(m_frustumCornersVS, m_nearClipPlane, m_farClipPlane);

		dx::XMStoreFloat4(&m_gpuFrustumPlaneDirVS, m_frustumVS.GetGPUFrustumPlaneDir());
	}

	void Camera::UpdateFrustumWS()
	{
		m_frustumWS.UpdatePlanesFromMatrix(GetViewProjectionMatrix());
	}

	void Camera::DrawImguiControlWindow()
	{
		if (ImGui::Begin("Camera"))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("R", &m_radius, 0.0f, 80.0f, "%.1f");
			ImGui::SliderAngle("Yaw", &m_orbitYaw, -180.0f, 180.0f);
			ImGui::SliderAngle("Pitch", &m_orbitPitch, -89.0f, 89.0f);

			ImGui::Text("Settings");
			float newFov = m_fov;
			ImGui::SliderFloat("FOV", &newFov, 10.0f, 180.0f, "%.1f");
			if (std::abs(newFov - m_fov) > FLT_EPSILON)
			{
				SetFOV(newFov);
			}
		}
		ImGui::End();
	}

	void Camera::Update(const u32 frameCt)
	{
		m_rotationWS = dx::XMFLOAT3(m_orbitPitch, -m_orbitYaw, 0.f);

		const auto camRotationMatrix = dx::XMMatrixRotationRollPitchYawFromVector(dx::XMLoadFloat3(&m_rotationWS));
		auto camTarget = XMVector3TransformCoord(dx::XMVectorSet(0.f, 0.f, 1.f, 0.f), camRotationMatrix);
		camTarget = dx::XMVector3Normalize(camTarget);

		// Update view and viewProj matrices (projection only changes when camera properties change)

		// Apply look-at and local orientation
		// +Y = up
		m_viewMatrix = dx::XMMatrixLookAtLH(GetPositionWS(), camTarget, dx::XMVectorSet(0.f, 1.f, 0.f, 0.f));
		m_viewProjectionMatrix = dx::XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());

		m_inverseViewMatrix = dx::XMMatrixInverse(nullptr, m_viewMatrix);
		m_inverseViewProjectionMatrix = dx::XMMatrixInverse(nullptr, m_viewProjectionMatrix);

		// Need to update this every frame too
		UpdateFrustumWS();
		UpdateBasisVectors();

		/*m_forwardWS = dx::XMVectorSetW(dx::XMVector3Transform(
			dx::XMVectorSet(0.f, 0.f, 1.f, 0.f),
			dx::XMMatrixRotationRollPitchYaw(m_orbitPitch, -m_orbitYaw, 0.0f) // rotate that offset
		), 0.f);*/

		// LookAt fails if position = target
		const auto safeRad = dx::XMMax(m_radius, 0.01f);
		SetPositionWS(dx::XMVectorSetW(dx::XMVectorScale(GetForwardWS(), -safeRad), 1.f));
	}
}