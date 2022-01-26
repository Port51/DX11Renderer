#include "pch.h"
#include "DirectionalLight.h"
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
#include "LightShadowData.h"
#include "Frustum.h"
#include "DrawContext.h"
#include "Renderer.h"
#include "RendererList.h"
#include "Config.h"
#include "Transforms.h"

namespace gfx
{
	DirectionalLight::DirectionalLight(Graphics& gfx, UINT index, float pan, float tilt, dx::XMFLOAT3 color, float intensity, float sphereRad, float range)
		: Light(gfx, index, dx::XMFLOAT3(0.f, 0.f, 0.f), color, intensity),
		pan(pan),
		tilt(tilt),
		sphereRad(sphereRad),
		range(range)
	{
		// todo: set shadow via settings
		shadowSettings.hasShadow = true;

		if (shadowSettings.hasShadow)
		{
			lightShadowData.resize(Config::ShadowCascades);
			shadowCascadeSpheresVS.resize(Config::ShadowCascades);
		}
	}

	void DirectionalLight::DrawImguiControlWindow()
	{
		const auto identifier = std::string("Light") + std::to_string(index);
		if (ImGui::Begin(identifier.c_str()))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("Pan", &pan, -360.0f, 360.0f, "%.1f");
			ImGui::SliderFloat("Tilt", &tilt, -180.0f, 180.0f, "%.1f");

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

	LightData DirectionalLight::GetLightData(dx::XMMATRIX viewMatrix) const
	{
		LightData light;
		const auto posWS_Vector = dx::XMLoadFloat4(&dx::XMFLOAT4(positionWS.x, positionWS.y, positionWS.z, 1.0f));
		const auto dirWS_Vector = dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(tilt), dx::XMConvertToRadians(pan), 0.0f));
		light.positionVS_range = dx::XMVectorSetW(dx::XMVector4Transform(posWS_Vector, viewMatrix), range); // pack range into W
		light.color_intensity = dx::XMVectorSetW(dx::XMLoadFloat3(&color), intensity);
		light.directionVS = dx::XMVectorSetW(dx::XMVector4Transform(dirWS_Vector, viewMatrix), (float)shadowAtlasTileIdx);
		light.data0 = dx::XMVectorSet(2, 1.f / sphereRad, 0, 0);
		return light;
	}

	UINT DirectionalLight::GetLightType() const
	{
		return 2u;
	}

	ViewProjTransforms DirectionalLight::GetShadowTransforms(dx::XMVECTOR cascadeSphereCenterWS, float cascadeDistance) const
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

	void DirectionalLight::RenderShadow(ShadowPassContext context)
	{
		const auto cameraPositionWS = context.pCamera->GetPositionWS();
		const auto cameraForwardWS = context.pCamera->GetForwardWS();

		const auto shadowDirWS = GetDirectionWS();

		// Render all cascades
		for (UINT i = 0u; i < Config::ShadowCascades; ++i)
		{
			const float sphereOffset = 5.f; // distance to gradually move spheres backwards along camera forward
			const float currentOffset = i * sphereOffset / (Config::ShadowCascades - 1);
			const float cascadeDistance = Config::ShadowCascadeDistances[i];
			auto cascadeSphereCenterWS = dx::XMVectorAdd(cameraPositionWS, dx::XMVectorScale(cameraForwardWS, cascadeDistance * 0.5f - currentOffset));

			// todo: calculate initial VP here

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

				const auto viewProjMatrix = initialTransforms.viewMatrix;// *initialTransforms.projMatrix;
				const auto projectedCenter = dx::XMVector4Transform(dx::XMVectorSetW(cascadeSphereCenterWS, 1.f), viewProjMatrix);
				const float x = std::floor(dx::XMVectorGetX(projectedCenter) / invTexelScale) / texelScale;
				const float y = std::floor(dx::XMVectorGetY(projectedCenter) / invTexelScale) / texelScale;
				const float z = dx::XMVectorGetZ(projectedCenter);
				dx::XMVECTOR correctedCenterWS = dx::XMVector4Transform(dx::XMVectorSet(x, y, z, 1.0f), dx::XMMatrixInverse(nullptr, viewProjMatrix));
				cascadeSphereCenterWS = correctedCenterWS;
				//cascadeSphereCenterWS = dx::XMVectorSet(0, 0, 0, 1);
			}

			// Calculate transforms that use stabilized sphere center
			ViewProjTransforms transforms = GetShadowTransforms(cascadeSphereCenterWS, cascadeDistance);

			// Record shadow sphere in VS
			shadowCascadeSpheresVS[i] = dx::XMVectorSetW(dx::XMVector3Transform(cascadeSphereCenterWS, context.pCamera->GetViewMatrix()), cascadeDistance * cascadeDistance * 0.25f); // 0.25 is because cascadeDistance is a diameter

			static Frustum frustum;

			// Setup transformation buffer
			static GlobalTransformCB transformationCB;
			transformationCB.viewMatrix = transforms.viewMatrix;
			transformationCB.projMatrix = transforms.projMatrix;
			context.pTransformationCB->Update(context.gfx, transformationCB);

			static DrawContext drawContext(context.renderer, context.renderer.ShadowPassName);
			drawContext.viewMatrix = transforms.viewMatrix;
			drawContext.projMatrix = transforms.projMatrix;

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
				lightShadowData[i].shadowMatrix = context.invViewMatrix * transforms.viewMatrix * transforms.projMatrix;
				dx::XMStoreUInt2(&lightShadowData[i].tile, dx::XMVectorSet(tileX, tileY, 0, 0));
			}
		}
	}

	void DirectionalLight::AppendShadowData(UINT shadowStartSlot, std::vector<LightShadowData>& shadowData) const
	{
		for (UINT i = 0u; i < Config::ShadowCascades; ++i)
		{
			shadowData[shadowStartSlot + i] = lightShadowData[i];
		}
	}

	UINT DirectionalLight::GetShadowTileCount() const
	{
		return HasShadow() ? Config::ShadowCascades : 0u;
	}

	dx::XMVECTOR DirectionalLight::GetShadowCascadeSphereVS(UINT idx) const
	{
		return shadowCascadeSpheresVS[idx];
	}

	dx::XMVECTOR DirectionalLight::GetDirectionWS() const
	{
		return dx::XMVector4Transform(dx::XMVectorSet(0, 0, 1, 0), dx::XMMatrixRotationRollPitchYaw(dx::XMConvertToRadians(tilt), dx::XMConvertToRadians(pan), 0.0f));
	}
}