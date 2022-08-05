#include "pch.h"
#include "GameObject.h"

namespace gfx
{
	void GameObject::SetPositionWS(const dx::XMVECTOR& positionWS)
	{
		dx::XMStoreFloat3(&m_positionWS, positionWS);
	}

	const dx::XMVECTOR& GameObject::GetPositionWS() const
	{
		return dx::XMLoadFloat3(&m_positionWS);
	}

	const dx::XMVECTOR& GameObject::GetForwardWS() const
	{
		return dx::XMLoadFloat3(&m_forwardWS);
	}

	const dx::XMVECTOR& GameObject::GetRightWS() const
	{
		return dx::XMLoadFloat3(&m_rightWS);
	}

	const dx::XMVECTOR& GameObject::GetUpWS() const
	{
		return dx::XMLoadFloat3(&m_upWS);
	}

	void GameObject::UpdateBasisVectors()
	{
		const auto rotMatrix = dx::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, m_roll);
		const auto forwardWS = dx::XMVector3Transform(dx::XMVectorSet(0.f, 0.f, 1.f, 0.f), rotMatrix);
		const auto rightWS = dx::XMVector3Transform(dx::XMVectorSet(1.f, 0.f, 0.f, 0.f), rotMatrix);
		const auto upWS = dx::XMVector3Transform(dx::XMVectorSet(0.f, 1.f, 0.f, 0.f), rotMatrix);

		dx::XMStoreFloat3(&m_forwardWS, forwardWS);
		dx::XMStoreFloat3(&m_rightWS, rightWS);
		dx::XMStoreFloat3(&m_upWS, upWS);
	}
}