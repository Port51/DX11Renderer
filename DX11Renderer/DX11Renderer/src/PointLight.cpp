#include "pch.h"
#include "PointLight.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
#include "ModelAsset.h"
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
#include "LightShadowData.h"
#include "RendererList.h"
#include "Config.h"

namespace gfx
{
	// The order is direction, up, direction up, ...
	std::vector<dx::XMVECTOR> PointLight::viewDirectionsWS = std::vector<dx::XMVECTOR>{
		dx::XMVectorSet(0, 0, 1, 0),
		dx::XMVectorSet(0, 1, 0, 0),
		dx::XMVectorSet(1, 0, 0, 0),
		dx::XMVectorSet(0, 1, 0, 0),
		dx::XMVectorSet(0, 0, -1, 0),
		dx::XMVectorSet(0, 1, 0, 0),
		dx::XMVectorSet(-1, 0, 0, 0),
		dx::XMVectorSet(0, 1, 0, 0),
		dx::XMVectorSet(0, 1, 0, 0),
		dx::XMVectorSet(-1, 0, 0, 0),
		dx::XMVectorSet(0, -1, 0, 0),
		dx::XMVectorSet(1, 0, 0, 0),
	};

	PointLight::PointLight(Graphics& gfx, UINT index, bool allowUserControl, bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, dx::XMFLOAT3 positionWS, dx::XMFLOAT3 color, float intensity, float attenuationQ, float range)
		: Light(gfx, index, allowUserControl, pModelAsset, positionWS, color, intensity),
		attenuationQ(attenuationQ),
		range(range)
	{
		shadowSettings.hasShadow = hasShadow;

		if (shadowSettings.hasShadow)
		{
			lightShadowData.resize(6);
		}
	}

	void PointLight::DrawImguiControlWindow()
	{
		if (!allowUserControl)
			return;

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
			ImGui::SliderFloat("Range", &range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Attenuation Q", &attenuationQ, 1.0f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
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

		// Precalculate sphere radius
		// (The shader math is the same, but it's easier to tune lights this way)
		float invSphereRad = attenuationQ / std::sqrtf(range);

		const auto posWS_Vector = dx::XMLoadFloat4(&dx::XMFLOAT4(positionWS.x, positionWS.y, positionWS.z, 1.0f));
		light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), range); // pack range into W
		light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&color), intensity);
		light.directionVS = dx::XMVectorSet(0, 0, 0, (float)shadowAtlasTileIdx);
		light.data0 = dx::XMVectorSet(0, invSphereRad, 0, 0);
		return light;
	}

	UINT PointLight::GetLightType() const
	{
		return 0u;
	}

	void PointLight::RenderShadow(ShadowPassContext context)
	{
		static const float fovTheta = (float)dx::XM_PI / 2.0f;
		static const float nearPlane = 0.1f;
		const auto projMatrix = dx::XMMatrixPerspectiveFovLH(fovTheta, 1.0f, nearPlane, range);

		// Render x6 shadow tiles
		for (UINT i = 0u; i < 6u; ++i)
		{
			// Apply look-at and local orientation
			// +Y = up
			const auto lightPos = dx::XMLoadFloat3(&positionWS);
			const auto viewMatrix = dx::XMMatrixLookAtLH(lightPos, dx::XMVectorAdd(lightPos, viewDirectionsWS[i * 2u + 0u]), viewDirectionsWS[i * 2u + 1u]);

			static Frustum frustum;

			// Setup transformation buffer
			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = viewMatrix;
			transformationCB.projMatrix = projMatrix;
			context.pTransformationCB->Update(context.gfx, transformationCB);

			static DrawContext drawContext(context.renderer, context.renderer.ShadowPassName);
			drawContext.viewMatrix = viewMatrix;
			drawContext.projMatrix = projMatrix;

			// This means all shadow draw calls need to be setup on the same thread
			context.pRendererList->Filter(frustum, RendererList::RendererSorting::FrontToBack);
			context.pRendererList->SubmitDrawCalls(drawContext);
			auto ct = context.pRendererList->GetRendererCount();

			// Calculate tile in shadow atlas
			int tileIdx = shadowAtlasTileIdx + i;
			int tileX = (tileIdx % Config::ShadowAtlasTileDimension);
			int tileY = (tileIdx / Config::ShadowAtlasTileDimension);

			// todo: defer the rendering
			{
				// Render to tile in atlas using viewport
				context.gfx.SetViewport(tileX * Config::ShadowAtlasTileResolution, tileY * Config::ShadowAtlasTileResolution, Config::ShadowAtlasTileResolution, Config::ShadowAtlasTileResolution);
				context.pRenderPass->Execute(context.gfx);
				context.pRenderPass->Reset(); // required to handle multiple shadows at once
			}

			// todo: move elsewhere
			{
				lightShadowData[i].shadowMatrix = context.invViewMatrix * viewMatrix * projMatrix;
				dx::XMStoreUInt2(&lightShadowData[i].tile, dx::XMVectorSet(tileX, tileY, 0, 0));
			}
		}
	}

	void PointLight::AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const
	{
		for (UINT i = 0u; i < 6u; ++i)
		{
			shadowData[shadowStartSlot + i] = lightShadowData[i];
		}
	}

	UINT PointLight::GetShadowTileCount() const
	{
		return HasShadow() ? 6u : 0u;
	}
}