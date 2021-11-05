#include "PointLight.h"
#include "imgui/imgui.h"
#include "FBXImporter.h"
#include <d3d11.h>

PointLight::PointLight(Graphics& gfx, DirectX::XMFLOAT3 position, float intensity, float radius)
	:
	cbuf(gfx),
	initialPositionWS(position),
	initialIntensity(intensity)
{
	Reset();

	auto pModelAsset = FBXImporter::LoadFBX("Models\\DefaultSphere.fbx", FBXImporter::FBXNormalsMode::Import, false);
	pModel = std::make_unique<ModelInstance>(gfx, pModelAsset, DirectX::XMFLOAT3{ 1.f, 1.f, 1.f }, DirectX::XMMatrixIdentity());
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
		ImGui::SliderFloat("Intensity", &cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::ColorEdit3("Diffuse Color", &cbData.diffuseColor.x);
		ImGui::ColorEdit3("Ambient", &cbData.ambient.x);

		ImGui::Text("Falloff");
		ImGui::SliderFloat("Constant", &cbData.attConst, 0.05f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Linear", &cbData.attLin, 0.0001f, 4.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Quadratic", &cbData.attQuad, 0.0000001f, 10.0f, "%.7f", ImGuiSliderFlags_Logarithmic);

		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}

void PointLight::Reset()
{
	positionWS = initialPositionWS;
	cbData = {
		{ positionWS.x, positionWS.y, positionWS.z },
		{ 0.05f, 0.05f, 0.05f },
		{ 1.0f, 1.0f, 1.0f },
		initialIntensity,
		1.0f,
		0.045f,
		0.0075f,
	};
}

void PointLight::Draw(Graphics& gfx) const
{
	pModel->SetPositionWS(positionWS);
	pModel->Draw(gfx);
}

void PointLight::Bind(Graphics& gfx, DirectX::FXMMATRIX viewMatrix) const
{
	auto dataCopy = cbData;
	const auto posWS_Vector = DirectX::XMLoadFloat3(&positionWS);

	// Transform WS to VS
	DirectX::XMStoreFloat3(&dataCopy.posVS, DirectX::XMVector3Transform(posWS_Vector, viewMatrix));

	cbuf.Update(gfx, PointLightCBuf{ dataCopy });
	cbuf.Bind(gfx);
}