#pragma once
#include "GameObject.h"
#include "Frustum.h"

namespace gfx
{

	class Camera : public GameObject
	{
	public:
		Camera(const float fov, const float aspect, const float nearClipPlane, const float farClipPlane);
		virtual ~Camera();
	public:
		const dx::XMMATRIX& GetViewMatrix() const;
		const dx::XMMATRIX& GetInverseViewMatrix() const;
		const dx::XMMATRIX& GetViewProjectionMatrix() const;
		const dx::XMMATRIX& GetInverseViewProjectionMatrix() const;
		const dx::XMMATRIX& GetProjectionMatrix() const;
		const dx::XMMATRIX& GetInverseProjectionMatrix() const;
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
		void Update();
	private:
		void UpdateProjectionMatrix();
		void UpdateFrustumVS();
		void UpdateFrustumWS();
	private:
		float m_nearClipPlane;
		float m_farClipPlane;
		float m_radius = 45.0f; // dist from origin
		float m_orbitYaw = 0.523599f; // rotate around origin
		float m_orbitPitch = 0.261799f; // rotate around origin
		float m_fov = 55.0f;
		float m_aspect = 0.75f;
		dx::XMMATRIX m_viewMatrix;
		dx::XMMATRIX m_inverseViewMatrix;
		dx::XMMATRIX m_viewProjectionMatrix;
		dx::XMMATRIX m_inverseViewProjectionMatrix;
		dx::XMMATRIX m_projectionMatrix;
		dx::XMMATRIX m_inverseProjectionMatrix;
		Frustum m_frustumWS;
		Frustum m_frustumVS;
		dx::XMFLOAT4 m_frustumCornersVS;
		dx::XMFLOAT4 m_inverseFrustumCornersVS;
		dx::XMFLOAT4 m_gpuFrustumPlaneDirVS;
	};
}