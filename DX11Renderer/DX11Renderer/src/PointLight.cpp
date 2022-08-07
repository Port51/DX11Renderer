#include "pch.h"
#include "PointLight.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "ModelAsset.h"
#include "LightData.h"
#include "Camera.h"
#include "DepthStencilTarget.h"
#include "RenderPass.h"
#include "ShadowPassContext.h"
#include "ConstantBuffer.h"
#include "GraphicsDevice.h"
#include "Frustum.h"
#include "DrawContext.h"
#include "Renderer.h"
#include "LightShadowData.h"
#include "RendererList.h"
#include "Config.h"
#include "RenderConstants.h"

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

	PointLight::PointLight(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, const bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMVECTOR positionWS, const dx::XMFLOAT3 color, const float intensity, const float attenuationQ, const float range)
		: Light(gfx, index, allowUserControl, pModelAsset, positionWS, color, intensity),
		m_attenuationQ(attenuationQ),
		m_range(range)
	{
		m_shadowSettings.hasShadow = hasShadow;

		if (m_shadowSettings.hasShadow)
		{
			lightShadowData.resize(6);
		}
	}

	void PointLight::DrawImguiControlWindow()
	{
		if (!m_allowUserControl)
			return;

		const auto identifier = std::string("Light") + std::to_string(m_index);
		if (ImGui::Begin(identifier.c_str()))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("X", &m_positionWS.x, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Y", &m_positionWS.y, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Z", &m_positionWS.z, -60.0f, 60.0f, "%.1f");

			ImGui::Text("Intensity/Color");
			// ImGuiSliderFlags_Logarithmic makes it power of 2?
			ImGui::SliderFloat("Intensity", &m_intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Range", &m_range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Attenuation Q", &m_attenuationQ, 1.0f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
			ImGui::ColorEdit3("Diffuse Color", &m_color.x);
		}

		if (m_pModel != nullptr)
		{
			m_pModel->SetPositionWS(dx::XMLoadFloat3(&m_positionWS));
		}
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

	const LightData PointLight::GetLightData(const dx::XMMATRIX& viewMatrix) const
	{
		LightData light;

		// Precalculate sphere radius
		// (The shader math is the same, but it's easier to tune lights this way)
		const float invSphereRad = m_attenuationQ / std::sqrtf(m_range);

		const auto posWS_Vector = dx::XMVectorSet(m_positionWS.x, m_positionWS.y, m_positionWS.z, 1.0f);
		light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), m_range); // pack range into W
		light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&m_color), m_intensity);
		light.directionVS = dx::XMVectorSet(0, 0, 0, (float)m_shadowAtlasTileIdx);
		light.data0 = dx::XMVectorSet(0, invSphereRad, 0, 0);
		return light;
	}

	const UINT PointLight::GetLightType() const
	{
		return 0u;
	}

	void PointLight::RenderShadow(const ShadowPassContext& context, RenderState& renderState)
	{
		static const float fovTheta = (float)dx::XM_PI / 2.0f;
		static const float nearPlane = 0.1f;
		const auto projMatrix = dx::XMMatrixPerspectiveFovLH(fovTheta, 1.0f, nearPlane, m_range);

		// Render x6 shadow tiles
		for (UINT i = 0u; i < 6u; ++i)
		{
			// Apply look-at and local orientation
			// +Y = up
			const auto lightPosWS = dx::XMLoadFloat3(&m_positionWS);
			const auto viewMatrix = dx::XMMatrixLookAtLH(lightPosWS, dx::XMVectorAdd(lightPosWS, viewDirectionsWS[i * 2u + 0u]), viewDirectionsWS[i * 2u + 1u]);

			static Frustum frustum;
			frustum.UpdatePlanesFromMatrix(viewMatrix * projMatrix);

			// Setup transformation buffer
			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = viewMatrix;
			transformationCB.projMatrix = projMatrix;
			context.transformationCB.Update(context.gfx, transformationCB);

			static DrawContext drawContext(context.renderer, RenderPassType::ShadowRenderPass);
			drawContext.viewMatrix = viewMatrix;
			drawContext.projMatrix = projMatrix;

			// This means all shadow draw calls need to be setup on the same thread
			context.pRendererList->Filter(context.gfx, frustum, RendererList::RendererSortingType::StateThenFrontToBack, RenderPassType::ShadowRenderPass, lightPosWS, viewDirectionsWS[i * 2u + 0u], m_range);
			context.pRendererList->SubmitDrawCalls(context.gfx, drawContext);
			const auto ct = context.pRendererList->GetRendererCount();

			// Calculate tile in shadow atlas
			const int tileIdx = m_shadowAtlasTileIdx + i;
			const int tileX = (tileIdx % Config::ShadowAtlasTileDimension);
			const int tileY = (tileIdx / Config::ShadowAtlasTileDimension);

			// todo: defer the rendering
			{
				// Render to tile in atlas using viewport
				context.gfx.SetViewport(tileX * Config::ShadowAtlasTileResolution, tileY * Config::ShadowAtlasTileResolution, Config::ShadowAtlasTileResolution, Config::ShadowAtlasTileResolution);
				context.renderPass.Execute(context.gfx, renderState);
				context.renderPass.Reset(); // required to handle multiple shadows at once
			}

			// todo: move elsewhere
			{
				lightShadowData[i].shadowMatrix = context.camera.GetInverseViewMatrix() * viewMatrix * projMatrix;
				dx::XMStoreUInt2(&lightShadowData[i].tile, dx::XMVectorSet(tileX, tileY, 0, 0));
			}
		}
	}

	void PointLight::AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const
	{
		for (UINT i = 0u; i < 6u; ++i)
		{
			shadowData[shadowStartSlot + i] = lightShadowData[i];
		}
	}

	const UINT PointLight::GetShadowTileCount() const
	{
		return HasShadow() ? 6u : 0u;
	}
}