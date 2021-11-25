#include "PointLight.h"
#include "imgui/imgui.h"
#include "FBXImporter.h"
#include <d3d11.h>
#include "ModelNode.h"
#include "MeshRenderer.h"

PointLight::PointLight(Graphics& gfx, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity, float range)
	: globalLightCbuf(gfx, "Light0"),
	positionWS(positionWS),
	color(color),
	range(range),
	intensity(intensity)
{

	auto pModelAsset = FBXImporter::LoadFBX(gfx.log, "Assets\\Models\\DefaultSphere.asset", FBXImporter::FBXNormalsMode::Import, false);
	pModel = std::make_unique<ModelInstance>(gfx, pModelAsset, dx::XMMatrixIdentity());
}

void PointLight::DrawImguiControlWindow()
{
	if (ImGui::Begin("Light"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &positionWS.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &positionWS.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &positionWS.z, -60.0f, 60.0f, "%.1f");

		ImGui::Text("Intensity/Color");
		// ImGuiSliderFlags_Logarithmic makes it power of 2?
		ImGui::SliderFloat("Intensity", &intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Range", &range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::ColorEdit3("Diffuse Color", &color.x);
	}
	ImGui::End();
}

void PointLight::SubmitDrawCalls(std::unique_ptr<FrameCommander>& frame) const
{
	pModel->SetPositionWS(positionWS);
	pModel->SubmitDrawCalls(frame);
}

void PointLight::Bind(Graphics& gfx, dx::FXMMATRIX viewMatrix) const
{
	auto dataCopy = cbData;
	const auto posWS_Vector = dx::XMLoadFloat3(&positionWS);

	// Transform WS to VS
	dx::XMStoreFloat3(&dataCopy.positionVS, dx::XMVector3Transform(posWS_Vector, viewMatrix));
	dataCopy.color = color;
	dataCopy.intensity = intensity;
	dataCopy.invRangeSqr = 1.f / std::max(range * range, 0.0001f);

	globalLightCbuf.Update(gfx, PointLightCBuf{ dataCopy });
	globalLightCbuf.BindPS(gfx, 0u);
}