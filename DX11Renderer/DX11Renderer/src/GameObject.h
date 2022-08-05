#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class GameObject
	{
	public:
		void SetPositionWS(const dx::XMVECTOR& positionWS);
		void SetTRS(const dx::XMVECTOR& positionWS, const dx::XMVECTOR& rotationWS, const dx::XMVECTOR& scaleWS);
		const dx::XMVECTOR& GetPositionWS() const;
		const dx::XMVECTOR& GetForwardWS() const;
		const dx::XMVECTOR& GetRightWS() const;
		const dx::XMVECTOR& GetUpWS() const;
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