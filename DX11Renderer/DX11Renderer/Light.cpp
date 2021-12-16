#include "Light.h"
#include "FBXImporter.h"

Light::Light(Graphics & gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity)
	: index(index),
	positionWS(positionWS),
	color(color),
	intensity(intensity)
{
	auto pModelAsset = FBXImporter::LoadFBX(gfx.GetLog(), "Assets\\Models\\DefaultSphere.asset", FBXImporter::FBXNormalsMode::Import, false);
	pModel = std::make_unique<ModelInstance>(gfx, pModelAsset, dx::XMMatrixIdentity());
}

void Light::SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const
{
	pModel->SetPositionWS(positionWS);
	pModel->SubmitDrawCalls(frame);
}
