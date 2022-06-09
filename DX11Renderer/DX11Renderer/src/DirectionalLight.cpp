#include "pch.h"
#include "DirectionalLight.h"
#include "DX11Include.h"
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
#include "LightShadowData.h"
#include "Frustum.h"
#include "DrawContext.h"
#include "Renderer.h"
#include "RendererList.h"
#include "Config.h"
#include "Transforms.h"
#include "RenderConstants.h"

namespace gfx
{
	DirectionalLight::DirectionalLight(const GraphicsDevice& gfx, const UINT index, const bool allowUserControl, const bool hasShadow, std::shared_ptr<ModelAsset> const& pModelAsset, const float pan, const float tilt, const dx::XMFLOAT3 color, const float intensity, const float sphereRad, const float range)
		: Light(gfx, index, allowUserControl, pModelAsset, dx::XMFLOAT3(0.f, 0.f, 0.f), color, intensity),
		m_pan(pan),
		m_tilt(tilt),
		m_sphereRad(sphereRad),
		m_range(range)
	{
		m_shadowSettings.hasShadow = hasShadow;

		if (m_shadowSettings.hasShadow)
		{
			m_lightShadowData.resize(Config::ShadowCascades);
			m_shadowCascadeSpheresVS.resize(Config::ShadowCascades);
		}
	}

	void DirectionalLight::DrawImguiControlWindow()
	{
		if (!m_allowUserControl)
			return;

		const auto identifier = std::string("Light") + std::to_string(m_index);
		if (ImGui::Begin(identifier.c_str()))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("Pan", &m_pan, -360.0f, 360.0f, "%.1f");
			ImGui::SliderFloat("Tilt", &m_tilt, -180.0f, 180.0f, "%.1f");

			ImGui::Text("Intensity/Color");
			// ImGuiSliderFlags_Logarithmic makes it power of 2?
			ImGui::SliderFloat("Intensity", &m_intensity, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("SphereRad", &m_sphereRad, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Range", &m_range, 0.05f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::ColorEdit3("Diffuse Color", &m_color.x);
		}

		m_pModel->SetPositionWS(m_positionWS);
		ImGui::End();
	}

	const LightData DirectionalLight::GetLightData(const dx::XMMATRIX viewMatrix) const
	{
		LightData light;
		const auto posWS_Vector = dx::XMVectorSet(m_positionWS.x, m_positionWS.y, m_positionWS.z, 1.0f);
		const auto dirWS_Vector = dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(m_tilt), dx::XMConvertToRadians(m_pan), 0.0f));
		light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), m_range); // pack range into W
		light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&m_color), m_intensity);
		light.directionVS = dx::XMVectorSetW(dx::XMVector4Transform(dirWS_Vector, viewMatrix), (float)m_shadowAtlasTileIdx);
		light.data0 = dx::XMVectorSet(2, 1.f / m_sphereRad, 0, 0);
		return light;
	}

	const UINT DirectionalLight::GetLightType() const
	{
		return 2u;
	}

	const ViewProjTransforms DirectionalLight::GetShadowTransforms(const dx::XMVECTOR cascadeSphereCenterWS, const float cascadeDistance) const
	{
		const float nearPlane = 0.5f;
		const auto cascadeFrustumStartWS = dx::XMVectorSubtract(cascadeSphereCenterWS, dx::XMVectorScale(GetDirectionWS(), cascadeDistance * 0.5f + Config::ShadowCascadeOffset + nearPlane));

		const auto viewMatrix = dx::XMMatrixLookAtLH(cascadeFrustumStartWS, cascadeSphereCenterWS, dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		// Add padding according to account for 1/2 of max "softness" kernel (like PCF)
		// In this case, 7-tap PCF
		const float padding = 1.f + 3.5f / (float)Config::ShadowAtlasTileResolution;
		const auto projMatrix = dx::XMMatrixOrthographicLH(cascadeDistance * padding, cascadeDistance * padding, nearPlane, cascadeDistance + Config::ShadowCascadeOffset * 1.01f + nearPlane);

		ViewProjTransforms transforms;
		transforms.viewMatrix = viewMatrix;
		transforms.projMatrix = projMatrix;
		return transforms;
	}

	void DirectionalLight::RenderShadow(const ShadowPassContext& context, RenderState& renderState)
	{
		const auto cameraPositionWS = context.camera.GetPositionWS();
		const auto cameraForwardWS = context.camera.GetForwardWS();

		const auto shadowDirWS = GetDirectionWS();

		// Render all cascades
		for (UINT i = 0u; i < (UINT)Config::ShadowCascades; ++i)
		{
			const float sphereOffset = 5.f; // distance to gradually move spheres backwards along camera forward
			const float currentOffset = i * sphereOffset / (Config::ShadowCascades - 1);
			const float cascadeDistance = Config::ShadowCascadeDistances[i];
			auto cascadeSphereCenterWS = dx::XMVectorAdd(cameraPositionWS, dx::XMVectorScale(cameraForwardWS, cascadeDistance * 0.5f - currentOffset));

			// Stabilize shadows so texel reads will match previous frame
			// References: 
			// - https://www.junkship.net/News/2020/11/22/shadow-of-a-doubt-part-2
			// - https://docs.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps
			// todo: move to separate method
			{
				const auto initialTransforms = GetShadowTransforms(cascadeSphereCenterWS, cascadeDistance);

				// Multiply by 2 since comparing to NDC
				const float texelScale = 2.f * (float)Config::ShadowAtlasTileResolution;
				const float invTexelScale = 1.f / texelScale;

				const auto viewProjMatrix = initialTransforms.viewMatrix * initialTransforms.projMatrix;
				const auto projectedCenter = dx::XMVector4Transform(dx::XMVectorSet(0.f, 0.f, 0.f, 1.f), viewProjMatrix);
				const float texelX = dx::XMVectorGetX(projectedCenter) * texelScale;
				const float texelY = dx::XMVectorGetY(projectedCenter) * texelScale;
				float xOffset = texelX - std::floor(texelX);
				if (xOffset > 0.5f) xOffset -= 1.f;
				float yOffset = texelY - std::floor(texelY);
				if (yOffset > 0.5f) yOffset -= 1.f;
				dx::XMVECTOR offsetWS = dx::XMVector4Transform(dx::XMVectorSet(xOffset * invTexelScale, yOffset * invTexelScale, 0.f, 1.f), dx::XMMatrixInverse(nullptr, viewProjMatrix));
				//cascadeSphereCenterWS = dx::XMVectorSubtract(cascadeSphereCenterWS, offsetWS);
			}

			// Calculate transforms that use stabilized sphere center
			const ViewProjTransforms transforms = GetShadowTransforms(cascadeSphereCenterWS, cascadeDistance);

			// Record shadow sphere in VS
			m_shadowCascadeSpheresVS[i] = dx::XMVectorSetW(dx::XMVector4Transform(dx::XMVectorSetW(cascadeSphereCenterWS, 1.f), context.camera.GetViewMatrix()), cascadeDistance * cascadeDistance * 0.25f); // 0.25 is because cascadeDistance is a diameter

			static Frustum frustum;

			// Setup transformation buffer
			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = transforms.viewMatrix;
			transformationCB.projMatrix = transforms.projMatrix;
			context.transformationCB.Update(context.gfx, transformationCB);

			static DrawContext drawContext(context.renderer, RenderPassType::ShadowRenderPass);
			drawContext.viewMatrix = transforms.viewMatrix;
			drawContext.projMatrix = transforms.projMatrix;

			// This means all shadow draw calls need to be setup on the same thread
			context.pRendererList->Filter(context.gfx, frustum, RendererList::RendererSortingType::StateThenFrontToBack, cascadeSphereCenterWS, shadowDirWS, cascadeDistance);
			context.pRendererList->SubmitDrawCalls(drawContext);
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
				m_lightShadowData[i].shadowMatrix = context.camera.GetInverseViewMatrix() * transforms.viewMatrix * transforms.projMatrix;
				dx::XMStoreUInt2(&m_lightShadowData[i].tile, dx::XMVectorSet((float)tileX, (float)tileY, 0, 0));
			}
		}
	}

	void DirectionalLight::AppendShadowData(const UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const
	{
		for (UINT i = 0u; i < (UINT)Config::ShadowCascades; ++i)
		{
			shadowData[shadowStartSlot + i] = m_lightShadowData[i];
		}
	}

	const UINT DirectionalLight::GetShadowTileCount() const
	{
		return HasShadow() ? Config::ShadowCascades : 0u;
	}

	const dx::XMVECTOR DirectionalLight::GetShadowCascadeSphereVS(const UINT idx) const
	{
		return m_shadowCascadeSpheresVS[idx];
	}

	const dx::XMVECTOR DirectionalLight::GetDirectionWS() const
	{
		return dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(m_tilt), dx::XMConvertToRadians(m_pan), 0.0f));
	}
}