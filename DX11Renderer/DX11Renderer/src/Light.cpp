#include "pch.h"
#include "Light.h"
#include "Model.h"
#include "ModelAsset.h"
#include "ConstantBuffer.h"
#include "GraphicsDevice.h"
#include "Config.h"

namespace gfx
{
	Light::Light(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMVECTOR positionWS, const dx::XMFLOAT3 color, const float intensity)
		: m_index(index),
		m_allowUserControl(allowUserControl),
		m_color(color),
		m_intensity(intensity),
		m_shadowAtlasTileIdx(-1),
		m_shadowSettings({ false })
	{
		SetPositionWS(positionWS);
		if (pModelAsset != nullptr)
		{
			const float scale = 0.25f;
			m_pModel = std::make_unique<Model>(gfx, pModelAsset, dx::XMMatrixScaling(scale, scale, scale) * dx::XMMatrixTranslationFromVector(positionWS));
		}
	}

	Light::~Light()
	{}

	const int Light::GetCurrentShadowIdx() const
	{
		return HasShadow() ? m_shadowAtlasTileIdx : -1;
	}

	void Light::SetCurrentShadowIdx(const int _shadowMapIdx)
	{
		m_shadowAtlasTileIdx = _shadowMapIdx;
	}

	const bool Light::HasShadow() const
	{
		return m_shadowSettings.hasShadow;
	}

	Model* Light::GetModel() const
	{
		return m_pModel.get();
	}

	void Light::SetShadowMatrixTile(dx::XMMATRIX& shadowMatrix, const int tileX, const int tileY)
	{
		// https://www.gamedev.net/forums/topic/591684-xna-40---shimmering-shadow-maps/#entry4752287
		// https://mynameismjp.wordpress.com/2009/02/17/deferred-cascaded-shadow-maps/

		// Applies scale and offset
		float scale = 1.f / Config::ShadowAtlasTileDimension;
		float baseOffset = (float)(Config::ShadowAtlasTileDimension - 1);
		float offsetX = -((float)tileX - baseOffset) * scale;
		float offsetY = -((float)tileY - baseOffset) * scale;
		return;
		shadowMatrix = shadowMatrix
			* dx::XMMatrixScaling(scale, -scale, 1.f)
			* dx::XMMatrixTranslation(offsetX, offsetY, 0.f);
	}
	const bool Light::AllowUserControl() const
	{
		return m_allowUserControl;
	}
}