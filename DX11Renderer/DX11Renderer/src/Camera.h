#pragma once
#include "GraphicsDevice.h"
#include "Frustum.h"

namespace gfx
{

	class Camera
	{
	public:
		Camera(const float fov, const float aspect, const float nearClipPlane, const float farClipPlane);
		virtual ~Camera() = default;
	public:
		const dx::XMVECTOR GetPositionWS() const;
		const dx::XMVECTOR GetForwardWS() const;
		const dx::XMMATRIX GetViewMatrix() const;
		const dx::XMMATRIX GetInverseViewMatrix() const;
		const dx::XMMATRIX GetViewProjectionMatrix() const;
		const dx::XMMATRIX GetInverseViewProjectionMatrix() const;
		const dx::XMMATRIX GetProjectionMatrix() const;
		const dx::XMMATRIX GetInverseProjectionMatrix() const;
		const dx::XMVECTOR GetFrustumCornersVS() const;
		const dx::XMVECTOR GetInverseFrustumCornersVS() const;
		const dx::XMVECTOR GetGPUFrustumPlaneDirVS() const;
		const Frustum& GetFrustumWS() const;
		const Frustum& GetFrustumVS() const;
		const float GetNearClipPlane() const;
		const float GetFarClipPlane() const;
		void SetFOV(const float fov);
		void SetAspect(const float aspect);
		void DrawImguiControlWindow();
		void Reset();
		void Update();
	private:
		void UpdateProjectionMatrix();
		void UpdateFrustumVS();
		void UpdateFrustumWS();
		void UpdateBasisWS();
	private:
		float m_nearClipPlane;
		float m_farClipPlane;
		float m_radius = 55.0f; // dist from origin
		float m_theta = 0.523599f; // rotate around origin
		float m_phi = 0.261799f; // rotate around origin
		float m_pitch = 0.0f;// 0.061f;
		float m_yaw = 0.0f;
		float m_roll = 0.0f;
		float m_fov = 40.0f;
		float m_aspect = 0.75f;
		dx::XMMATRIX m_viewMatrix;
		dx::XMMATRIX m_inverseViewMatrix;
		dx::XMMATRIX m_viewProjectionMatrix;
		dx::XMMATRIX m_inverseViewProjectionMatrix;
		dx::XMMATRIX m_projectionMatrix;
		dx::XMMATRIX m_inverseProjectionMatrix;
		Frustum m_frustumWS;
		Frustum m_frustumVS;
		dx::XMVECTOR m_frustumCornersVS;
		dx::XMVECTOR m_inverseFrustumCornersVS;
		dx::XMVECTOR m_gpuFrustumPlaneDirVS;
		dx::XMVECTOR m_forwardWS;
		dx::XMVECTOR m_positionWS;
	};
}