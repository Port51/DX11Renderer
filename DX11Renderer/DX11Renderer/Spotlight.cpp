#include "Spotlight.h"
#include "imgui/imgui.h"
#include "DX11Include.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
#include "LightData.h"
#include "Camera.h"
#include "DepthStencilTarget.h"
#include "RenderPass.h"
#include "ShadowPassContext.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "Frustum.h"
#include "DrawContext.h"
#include "Renderer.h"
#include "RendererList.h"

Spotlight::Spotlight(Graphics& gfx, UINT index, dx::XMFLOAT3 positionWS, float pan, float tilt, dx::XMFLOAT3 color, float intensity, float sphereRad, float range)
	: Light(gfx, index, positionWS, color, intensity),
	pan(pan),
	tilt(tilt),
	sphereRad(sphereRad),
	range(range)
{
	// todo: set shadow via settings
	shadowSettings.hasShadow = true;
	pShadowMap = std::make_unique<DepthStencilTarget>(gfx, 1024, 1024);

	pShadowPassCB = std::make_unique<ConstantBuffer<ShadowPassCB>>(gfx, D3D11_USAGE_DYNAMIC);
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
		ImGui::SliderFloat("Pan", &pan, -360.0f, 360.0f, "%.1f");
		ImGui::SliderFloat("Tilt", &tilt, -180.0f, 180.0f, "%.1f");

		ImGui::Text("Intensity/Color");
		// ImGuiSliderFlags_Logarithmic makes it power of 2?
		ImGui::SliderFloat("Intensity", &intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("SphereRad", &sphereRad, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Range", &range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("InnerCos", &innerCos, 0.0f, 1.0f, "%.01f");
		ImGui::SliderFloat("OuterCos", &outerCos, 0.0f, 1.0f, "%.01f");
		ImGui::ColorEdit3("Diffuse Color", &color.x);
	}

	pModel->SetPositionWS(positionWS);
	ImGui::End();
}

LightData Spotlight::GetLightData(dx::XMMATRIX viewMatrix) const
{
	LightData light;
	const auto posWS_Vector = dx::XMLoadFloat4(&dx::XMFLOAT4(positionWS.x, positionWS.y, positionWS.z, 1.0f));
	light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), range); // pack range into W
	light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&color), intensity);
	light.directionVS = dx::XMVector4Transform(GetDirectionWS(), viewMatrix);
	light.data0 = dx::XMVectorSet(1, 1.f / sphereRad, dx::XMMax(outerCos + 0.01f, innerCos), outerCos);
	return light;
}

UINT Spotlight::GetLightType() const
{
	return 1u;
}

void Spotlight::RenderShadow(ShadowPassContext context)
{
	// Apply look-at and local orientation
	// +Y = up
	const auto lightPos = dx::XMLoadFloat3(&positionWS);
	const auto viewMatrix = dx::XMMatrixLookAtLH(lightPos, dx::XMVectorAdd(lightPos, GetDirectionWS()), dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
		* dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(tilt), dx::XMConvertToRadians(-pan), 0.f);

	float fovTheta = 2.0f * std::acos(outerCos); // todo: store actual angle
	const float nearPlane = 0.1f;
	const auto projMatrix = dx::XMMatrixPerspectiveFovLH(fovTheta, 1.0f, nearPlane, range);

	Frustum frustum;

	// Setup transformation buffer
	TransformationCB transformationCB;
	transformationCB.viewMatrix = viewMatrix;
	transformationCB.projMatrix = projMatrix;
	context.pTransformationCB->Update(context.gfx, transformationCB);

	DrawContext drawContext(context.renderer);
	drawContext.viewMatrix = viewMatrix;
	drawContext.projMatrix = projMatrix;
	drawContext.SetRenderPasses(context.renderer.ShadowPassName);

	// This means all shadow draw calls need to be setup on the same thread
	context.pRendererList->Filter(frustum, RendererList::RendererSorting::FrontToBack);
	context.pRendererList->SubmitDrawCalls(drawContext);
	auto ct = context.pRendererList->GetRendererCount();

	// todo: defer the rendering
	{
		pShadowMap->Clear(context.gfx);
		context.gfx.GetContext()->OMSetRenderTargets(0, nullptr, pShadowMap->GetView().Get());
		context.gfx.SetViewport(1024, 1024);

		context.pRenderPass->Execute(context.gfx);
		context.pRenderPass->Reset(); // required to handle multiple shadows at once
	}
	
}

dx::XMVECTOR Spotlight::GetDirectionWS() const
{
	return dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(tilt), dx::XMConvertToRadians(pan), 0.0f));
}
