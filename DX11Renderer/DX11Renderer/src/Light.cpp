#include "pch.h"
#include "Light.h"
#include "FBXImporter.h"
#include "ModelInstance.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "Config.h"

namespace gfx
{
	Light::Light(Graphics & gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity)
		: index(index),
		positionWS(positionWS),
		color(color),
		intensity(intensity)
	{
		auto pModelAsset = FBXImporter::LoadFBX(gfx, "Assets\\Models\\DefaultSphere.asset", FBXImporter::FBXNormalsMode::Import, false);
		pModel = std::make_unique<ModelInstance>(gfx, pModelAsset, dx::XMMatrixIdentity());
	}

	Light::~Light()
	{}

	int Light::GetCurrentShadowIdx() const
	{
		return shadowMapIdx;
	}

	void Light::SetCurrentShadowIdx(int _shadowMapIdx)
	{
		shadowMapIdx = _shadowMapIdx + 1u;
	}

	bool Light::HasShadow() const
	{
		return shadowSettings.hasShadow;
	}

	ModelInstance & Light::GetModelInstance() const
	{
		return *pModel;
	}

	void Light::SetShadowMatrixTile(dx::XMMATRIX& shadowMatrix, int tileX, int tileY)
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
}