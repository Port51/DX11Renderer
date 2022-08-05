#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class GameObject
	{
	public:
		void SetPositionWS(const dx::XMVECTOR& positionWS, const bool updateTransform = true);
		void SetPitch(const float pitch, const bool updateTransform = true);
		void SetYaw(const float yaw, const bool updateTransform = true);
		void SetRoll(const float roll, const bool updateTransform = true);
		void SetRotationWS(const float pitch, const float yaw, const float roll, const bool updateTransform = true);
		void SetRotationWS(const dx::XMVECTOR& rotationWS, const bool updateTransform = true);
		void SetScaleWS(const dx::XMVECTOR& scaleWS, const bool updateTransform = true);
		void SetTRS(const dx::XMVECTOR& positionWS, const dx::XMVECTOR& rotationWS, const dx::XMVECTOR& scaleWS, const bool updateTransform = true);
		void UpdateTransform();

		const dx::XMVECTOR GetPositionWS() const;
		const dx::XMVECTOR GetForwardWS() const;
		const dx::XMVECTOR GetRightWS() const;
		const dx::XMVECTOR GetUpWS() const;
	protected:
		void UpdateBasisVectors();
		void DecomposeTRS();
		virtual void ApplyTRS();
	protected:
		// TRS data
		dx::XMFLOAT3 m_positionWS;
		dx::XMFLOAT3 m_scaleWS;
		dx::XMFLOAT3 m_rotationWS;
		//float m_pitch = 0.0f;
		//float m_yaw = 0.0f;
		//float m_roll = 0.0f;

		// Basis vectors
		dx::XMFLOAT3 m_forwardWS;
		dx::XMFLOAT3 m_rightWS;
		dx::XMFLOAT3 m_upWS;

		// Combined TRS
		dx::XMFLOAT4X4 m_transform;
	};
}