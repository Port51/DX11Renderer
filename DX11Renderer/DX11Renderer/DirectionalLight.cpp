#include "DirectionalLight.h"
#include "imgui/imgui.h"
#include <d3d11.h>
#include "MeshRenderer.h"
#include "LightData.h"
#include "Camera.h"
#include "DepthStencilTarget.h"
#include "RenderPass.h"

DirectionalLight::DirectionalLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity, float sphereRad, float range)
	: Light(gfx, index, positionWS, color, intensity),
	pan(pan),
	tilt(tilt),
	sphereRad(sphereRad),
	range(range)
{

}

void DirectionalLight::DrawImguiControlWindow()
{
	const auto identifier = std::string("Light") + std::to_string(index);
	if (ImGui::Begin(identifier.c_str()))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &positionWS.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &positionWS.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &positionWS.z, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Pan", &pan, -360.0f, 360.0f, "%.1f");
		ImGui::SliderFloat("Tilt", &tilt, -180.0f, 180.0f, "%.1f");

		ImGui::Text("Intensity/Color");
		// ImGuiSliderFlags_Logarithmic makes it power of 2?
		ImGui::SliderFloat("Intensity", &intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("SphereRad", &sphereRad, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Range", &range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::ColorEdit3("Diffuse Color", &color.x);
	}
	ImGui::End();
}

LightData DirectionalLight::GetLightData(dx::FXMMATRIX viewMatrix) const
{
	LightData light;
	const auto posWS_Vector = dx::XMLoadFloat4(&dx::XMFLOAT4(positionWS.x, positionWS.y, positionWS.z, 1.0f));
	const auto dirWS_Vector = dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(tilt), dx::XMConvertToRadians(pan), 0.0f));
	light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), range); // pack range into W
	light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&color), intensity);
	light.directionVS = dx::XMVector4Transform(dirWS_Vector, viewMatrix);
	light.data0 = dx::XMVectorSet(2, 1.f / sphereRad, 0, 0);
	return light;
}

void DirectionalLight::SubmitDrawCalls(std::unique_ptr<Renderer>& frame) const
{
	// Basically just a way to not render directional lights as an object
}

UINT DirectionalLight::GetLightType() const
{
	return 2u;
}

void DirectionalLight::RenderShadow(Graphics & gfx, const Camera & cam, const std::unique_ptr<RenderPass>& pass, const std::unique_ptr<ConstantBuffer<TransformationCB>>& pTransformationCB)
{}
