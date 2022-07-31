#include "pch.h"
#include "Spotlight.h"
#include "MeshRenderer.h"
#include "ModelInstance.h"
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
#include "RendererList.h"
#include "Config.h"
#include "RenderConstants.h"

namespace gfx
{
	Spotlight::Spotlight(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, const bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, const dx::XMFLOAT3 positionWS, const float pan, const float tilt, const dx::XMFLOAT3 color, const float intensity, const float attenuationQ, const float range)
		: Light(gfx, index, allowUserControl, pModelAsset, positionWS, color, intensity),
		m_pan(pan),
		m_tilt(tilt),
		m_attenuationQ(attenuationQ),
		m_range(range)
	{
		m_shadowSettings.hasShadow = hasShadow;

		if (m_shadowSettings.hasShadow)
		{
			m_pShadowPassCB = std::make_unique<ConstantBuffer<ShadowPassCB>>(gfx, D3D11_USAGE_DYNAMIC);
		}
	}

	void Spotlight::DrawImguiControlWindow()
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
			ImGui::SliderFloat("Pan", &m_pan, -360.0f, 360.0f, "%.1f");
			ImGui::SliderFloat("Tilt", &m_tilt, -180.0f, 180.0f, "%.1f");

			ImGui::Text("Intensity/Color");
			// ImGuiSliderFlags_Logarithmic makes it power of 2?
			ImGui::SliderFloat("Intensity", &m_intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Range", &m_range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Attenuation Q", &m_attenuationQ, 1.0f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("InnerAng", &m_innerAngle, 0.0f, 90.0f, "%.1f");
			ImGui::SliderFloat("OuterAng", &m_outerAngle, 0.0f, 90.0f, "%.1f");
			ImGui::ColorEdit3("Diffuse Color", &m_color.x);
		}

		if (m_pModel != nullptr)
		{
			m_pModel->SetPositionWS(m_positionWS);
		}
		ImGui::End();
	}

	const LightData Spotlight::GetLightData(const dx::XMMATRIX viewMatrix) const
	{
		LightData light;

		// Precalculate sphere radius
		// (The shader math is the same, but it's easier to tune lights this way)
		const float invSphereRad = m_attenuationQ / std::sqrtf(m_range);

		const auto posWS_Vector = dx::XMVectorSet(m_positionWS.x, m_positionWS.y, m_positionWS.z, 1.0f);
		light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), m_range); // pack range into W
		light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&m_color), m_intensity);
		light.directionVS = dx::XMVectorSetW(dx::XMVector4Transform(GetDirectionWS(), viewMatrix), (float)m_shadowAtlasTileIdx);
		light.data0 = dx::XMVectorSet(1, invSphereRad, dx::XMMax(std::cos(dx::XMConvertToRadians(m_outerAngle)) + 0.01f, std::cos(dx::XMConvertToRadians(m_innerAngle))), std::cos(dx::XMConvertToRadians(m_outerAngle)));
		return light;
	}

	const UINT Spotlight::GetLightType() const
	{
		return 1u;
	}

	void Spotlight::RenderShadow(const ShadowPassContext& context, RenderState& renderState)
	{
		// Apply look-at and local orientation
		// +Y = up
		const auto lightPosWS = dx::XMLoadFloat3(&m_positionWS);
		const auto viewMatrix = dx::XMMatrixLookAtLH(lightPosWS, dx::XMVectorAdd(lightPosWS, GetDirectionWS()), dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		float fovTheta = dx::XMConvertToRadians(2.0f * m_outerAngle);
		const float nearPlane = 0.1f;
		const auto projMatrix = dx::XMMatrixPerspectiveFovLH(fovTheta, 1.0f, nearPlane, m_range);

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
		context.pRendererList->Filter(context.gfx, frustum, RendererList::RendererSortingType::StateThenFrontToBack, lightPosWS, GetDirectionWS(), m_range);
		context.pRendererList->SubmitDrawCalls(drawContext);
		const auto ct = context.pRendererList->GetRendererCount();

		// Calculate tile in shadow atlas
		const int tileX = (m_shadowAtlasTileIdx % Config::ShadowAtlasTileDimension);
		const int tileY = (m_shadowAtlasTileIdx / Config::ShadowAtlasTileDimension);

		// todo: defer the rendering
		{
			// Render to tile in atlas using viewport
			context.gfx.SetViewport(tileX * Config::ShadowAtlasTileResolution, tileY * Config::ShadowAtlasTileResolution, Config::ShadowAtlasTileResolution, Config::ShadowAtlasTileResolution);
			context.renderPass.Execute(context.gfx, renderState);
			context.renderPass.Reset(); // required to handle multiple shadows at once
		}

		// todo: move elsewhere
		{
			m_lightShadowData.shadowMatrix = context.camera.GetInverseViewMatrix() * viewMatrix * projMatrix;
			dx::XMStoreUInt2(&m_lightShadowData.tile, dx::XMVectorSet(tileX, tileY, 0, 0));
			//SetShadowMatrixTile(lightShadowData.shadowMatrix, tileX, tileY);
		}

	}

	void Spotlight::AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const
	{
		shadowData[shadowStartSlot] = m_lightShadowData;
	}

	const UINT Spotlight::GetShadowTileCount() const
	{
		return HasShadow() ? 1u : 0u;
	}

	const dx::XMVECTOR Spotlight::GetDirectionWS() const
	{
		return dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(m_tilt), dx::XMConvertToRadians(m_pan), 0.0f));
	}
}