#include "pch.h"
#include "Light.h"
#include "FBXImporter.h"
#include "ModelInstance.h"
#include "ConstantBuffer.h"
#include "Graphics.h"

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

	bool Light::HasShadow() const
	{
		return shadowSettings.hasShadow;
	}

	ModelInstance & Light::GetModelInstance() const
	{
		return *pModel;
	}
}