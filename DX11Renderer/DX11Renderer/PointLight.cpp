#include "PointLight.h"
#include "imgui/imgui.h"

PointLight::PointLight(Graphics& gfx, DirectX::XMFLOAT3 position, float radius)
	:
	cbuf(gfx),
	initialPositionWS(position)
{
	Reset();
}

void PointLight::DrawImguiControlWindow()
{
	if (ImGui::Begin("Light"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &cbData.posVS.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &cbData.posVS.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &cbData.posVS.z, -60.0f, 60.0f, "%.1f");

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
	cbData = {
		{ initialPositionWS.x, initialPositionWS.y, initialPositionWS.z },
		{ 0.05f, 0.05f, 0.05f },
		{ 1.0f, 1.0f, 1.0f },
		0.41f,
		1.0f,
		0.045f,
		0.0075f,
	};
}

void PointLight::Draw(Graphics& gfx) const
{
	//mesh.SetPos(cbData.posVS);
	//mesh.Draw(gfx);
}

void PointLight::Bind(Graphics& gfx, DirectX::FXMMATRIX viewMatrix) const
{
	auto dataCopy = cbData;
	const auto pos = DirectX::XMLoadFloat3(&cbData.posVS);

	// Transform WS to VS
	DirectX::XMStoreFloat3(&dataCopy.posVS, DirectX::XMVector3Transform(pos, viewMatrix));

	cbuf.Update(gfx, PointLightCBuf{ dataCopy });
	cbuf.Bind(gfx);
}