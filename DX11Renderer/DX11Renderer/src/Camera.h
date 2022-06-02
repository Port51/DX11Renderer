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
		const dx::XMMATRIX GetViewProjectionMatrix() const;
		const dx::XMMATRIX GetProjectionMatrix() const;
		const dx::XMVECTOR GetFrustumCornersVS() const;
		const dx::XMVECTOR GetInverseFrustumCornersVS() const;
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
		float m_radius = 19.9f;// 10.0f;// 13.3f; // dist from origin
		float m_theta = -0.383972f;// -0.471225f; // rotate around origin
		float m_phi = 0.558505f;// 0.209f; // rotate around origin
		float m_pitch = 0.f;// 0.061f;
		float m_yaw = 0.0f;
		float m_roll = 0.0f;
		float m_fov = 40.0f;
		float m_aspect = 0.75f;
		dx::XMMATRIX m_viewMatrix;
		dx::XMMATRIX m_viewProjectionMatrix;
		dx::XMMATRIX m_projectionMatrix;
		Frustum m_frustumWS;
		Frustum m_frustumVS;
		dx::XMVECTOR m_frustumCornersVS;
		dx::XMVECTOR m_inverseFrustumCornersVS;
		dx::XMVECTOR m_forwardWS;
		dx::XMVECTOR m_positionWS;
	};
}