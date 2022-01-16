#include "pch.h"
#include "PointLight.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
#include "LightData.h"
#include "Camera.h"
#include "DepthStencilTarget.h"
#include "RenderPass.h"
#include "ShadowPassContext.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "LightShadowData.h"

namespace gfx
{
	PointLight::PointLight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity, float sphereRad, float range)
		: Light(gfx, index, positionWS, color, intensity),
		sphereRad(sphereRad),
		range(range)
	{

	}

	void PointLight::DrawImguiControlWindow()
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

		pModel->SetPositionWS(positionWS);
		ImGui::End();
	}

	/*void PointLight::Bind(Graphics& gfx, dx::XMMATRIX viewMatrix) const
	{
		auto dataCopy = cbData;
		const auto posWS_Vector = dx::XMLoadFloat3(&positionWS);

		// Transform WS to VS
		dx::XMStoreFloat3(&dataCopy.positionVS, dx::XMVector3Transform(posWS_Vector, viewMatrix));
		dataCopy.color = color;
		dataCopy.intensity = intensity;
		dataCopy.invRangeSqr = 1.f / std::max(range * range, 0.0001f);

		globalLightCbuf.Update(gfx, PointLightCBuf{ dataCopy });
		//globalLightCbuf.BindPS(gfx, 0u);
		gfx.GetContext()->PSSetConstantBuffers(0u, 1u, globalLightCbuf.GetD3DBuffer().GetAddressOf());
	}*/

	LightData PointLight::GetLightData(dx::XMMATRIX viewMatrix) const
	{
		LightData light;

		const auto posWS_Vector = dx::XMLoadFloat4(&dx::XMFLOAT4(positionWS.x, positionWS.y, positionWS.z, 1.0f));
		light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), range); // pack range into W
		light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&color), intensity);
		light.directionVS = dx::XMVectorSet(0, 0, 0, 0);
		light.data0 = dx::XMVectorSet(0, 1.f / sphereRad, 0, 0);
		return light;
	}

	UINT PointLight::GetLightType() const
	{
		return 0u;
	}

	void PointLight::RenderShadow(ShadowPassContext context)
	{}

	void PointLight::AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData, std::vector<ComPtr<ID3D11ShaderResourceView>>& srvs) const
	{}

	UINT PointLight::GetShadowSRVCount() const
	{
		return HasShadow() ? 6u : 0u;
	}
}