#include "pch.h"
#include "GameObject.h"

namespace gfx
{
	void GameObject::SetPositionWS(const dx::XMVECTOR& positionWS, const bool updateTransform)
	{
		dx::XMStoreFloat3(&m_positionWS, positionWS);
		if (updateTransform) ApplyTRS();
	}

	void GameObject::SetPitch(const float pitch, const bool updateTransform)
	{
		m_rotationWS.x = pitch;
		if (updateTransform) UpdateTransform();
	}

	void GameObject::SetYaw(const float yaw, const bool updateTransform)
	{
		m_rotationWS.y = yaw;
		if (updateTransform) UpdateTransform();
	}

	void GameObject::SetRoll(const float roll, const bool updateTransform)
	{
		m_rotationWS.z = roll;
		if (updateTransform) UpdateTransform();
	}

	void GameObject::SetRotationWS(const float pitch, const float yaw, const float roll, const bool updateTransform)
	{
		m_rotationWS = dx::XMFLOAT3(pitch, yaw, roll);
		if (updateTransform) UpdateTransform();
	}

	void GameObject::SetRotationWS(const dx::XMVECTOR& rotationWS, const bool updateTransform)
	{
		dx::XMStoreFloat3(&m_rotationWS, rotationWS);
		if (updateTransform) UpdateTransform();
	}

	void GameObject::SetScaleWS(const dx::XMVECTOR& scaleWS, const bool updateTransform)
	{
		dx::XMStoreFloat3(&m_scaleWS, scaleWS);
		if (updateTransform) ApplyTRS();
	}

	void GameObject::SetTRS(const dx::XMVECTOR& positionWS, const dx::XMVECTOR& rotationWS, const dx::XMVECTOR& scaleWS, const bool updateTransform)
	{
		dx::XMStoreFloat3(&m_positionWS, positionWS);
		dx::XMStoreFloat3(&m_rotationWS, rotationWS);
		dx::XMStoreFloat3(&m_scaleWS, scaleWS);
		if (updateTransform) UpdateTransform();
	}

	void GameObject::UpdateTransform()
	{
		ApplyTRS();
		UpdateBasisVectors();
	}

	const dx::XMVECTOR GameObject::GetPositionWS() const
	{
		return dx::XMLoadFloat3(&m_positionWS);
	}

	const dx::XMVECTOR GameObject::GetForwardWS() const
	{
		return dx::XMLoadFloat3(&m_forwardWS);
	}

	const dx::XMVECTOR GameObject::GetRightWS() const
	{
		return dx::XMLoadFloat3(&m_rightWS);
	}

	const dx::XMVECTOR GameObject::GetUpWS() const
	{
		return dx::XMLoadFloat3(&m_upWS);
	}

	void GameObject::UpdateBasisVectors()
	{
		const auto rotMatrix = dx::XMMatrixRotationRollPitchYawFromVector(dx::XMLoadFloat3(&m_rotationWS));
		const auto forwardWS = dx::XMVector3Transform(dx::XMVectorSet(0.f, 0.f, 1.f, 0.f), rotMatrix);
		const auto rightWS = dx::XMVector3Transform(dx::XMVectorSet(1.f, 0.f, 0.f, 0.f), rotMatrix);
		const auto upWS = dx::XMVector3Transform(dx::XMVectorSet(0.f, 1.f, 0.f, 0.f), rotMatrix);

		dx::XMStoreFloat3(&m_forwardWS, forwardWS);
		dx::XMStoreFloat3(&m_rightWS, rightWS);
		dx::XMStoreFloat3(&m_upWS, upWS);
	}

	void GameObject::DecomposeTRS()
	{
		auto trs = dx::XMLoadFloat4x4(&m_transform);
		dx::XMVECTOR tra;
		dx::XMVECTOR rot;
		dx::XMVECTOR sca;
		dx::XMMatrixDecompose(&sca, &rot, &tra, trs);
		dx::XMStoreFloat3(&m_positionWS, tra);
		dx::XMStoreFloat3(&m_rotationWS, rot);
		dx::XMStoreFloat3(&m_scaleWS, sca);
	}

	void GameObject::ApplyTRS()
	{
		auto trs = dx::XMMatrixScalingFromVector(dx::XMLoadFloat3(&m_scaleWS))
			* dx::XMMatrixRotationRollPitchYawFromVector(dx::XMLoadFloat3(&m_rotationWS))
			* dx::XMMatrixTranslationFromVector(dx::XMLoadFloat3(&m_positionWS));
		dx::XMStoreFloat4x4(&m_transform, trs);
	}
}