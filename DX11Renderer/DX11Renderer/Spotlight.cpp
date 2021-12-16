#include "Spotlight.h"
#include "imgui/imgui.h"
#include "FBXImporter.h"
#include <d3d11.h>
#include "MeshRenderer.h"
#include "LightData.h"

Spotlight::Spotlight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 rotation, dx::XMFLOAT3 color, float intensity, float sphereRad, float range)
	: index(index),
	positionWS(positionWS),
	rotation(rotation),
	color(color),
	sphereRad(sphereRad),
	range(range),
	intensity(intensity)
{

	auto pModelAsset = FBXImporter::LoadFBX(gfx.GetLog(), "Assets\\Models\\DefaultSphere.asset", FBXImporter::FBXNormalsMode::Import, false);
	pModel = std::make_unique<ModelInstance>(gfx, pModelAsset, dx::XMMatrixIdentity());
}

void Spotlight::DrawImguiControlWindow()
{
	const auto identifier = std::string("Light") + std::to_string(index);
	if (ImGui::Begin(identifier.c_str()))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &positionWS.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &positionWS.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &positionWS.z, -60.0f, 60.0f, "%.1f");

		ImGui::Text("Intensity/Color");
		// ImGuiSliderFlags_Logarithmic makes it power of 2?
		ImGui::SliderFloat("Intensity", &intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("SphereRad", &sphereRad, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Range", &range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::ColorEdit3("Diffuse Color", &color.x);
	}
	ImGui::End();
}

void Spotlight::SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const
{
	pModel->SetPositionWS(positionWS);
	pModel->SubmitDrawCalls(frame);
}

LightData Spotlight::GetLightData(dx::FXMMATRIX viewMatrix) const
{
	LightData light;

	const auto posWS_Vector = dx::XMLoadFloat4(&dx::XMFLOAT4(positionWS.x, positionWS.y, positionWS.z, 1.0f));
	light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), range); // pack range into W
	light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&color), intensity);
	light.data0 = dx::XMVectorSet(1, 1.f / sphereRad, 0, 0);
	return light;
}
