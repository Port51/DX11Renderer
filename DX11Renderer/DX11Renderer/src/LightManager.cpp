#include "pch.h"
#include "LightManager.h"

#include "Graphics.h"
#include "Camera.h"
#include "StructuredBuffer.h"
#include "Renderer.h"
#include "RenderConstants.h"
#include "PointLight.h"
#include "Spotlight.h"
#include "DirectionalLight.h"
#include "RenderTexture.h"
#include "RenderPass.h"
#include "RendererList.h"
#include "ModelInstance.h"
#include "LightShadowData.h"
#include "DepthStencilTarget.h"
#include "ModelImporter.h"
#include "Config.h"

namespace gfx
{
	LightManager::LightManager(Graphics & gfx, std::shared_ptr<RendererList> pRendererList)
	{
		auto pLightModelAsset = ModelImporter::LoadGLTF(gfx, "Assets\\Models\\DefaultSphere.asset");

		pLightData = std::make_unique<StructuredBuffer<LightData>>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, MaxLightCount);
		cachedLightData.resize(MaxLightCount);

		UINT lightIdx = 0u;
		pLights.emplace_back(std::make_shared<PointLight>(gfx, lightIdx++, true, true, pLightModelAsset, dx::XMFLOAT3(0.f, 2.5f, 0.f), dx::XMFLOAT3(1.f, 1.f, 1.f), 3.f, 3.f, 5.f));

		// Alignment bug:
		//pLights.emplace_back(std::make_shared<Spotlight>(gfx, lightIdx++, true, true, pLightModelAsset, dx::XMFLOAT3(8.5f, 1.5f, -2.0f), 0.0f, 0.0f, dx::XMFLOAT3(1.0f, 1.0f, 1.0f), 3.0f, 50.f, 50.f));
		//pMainLight = std::make_shared<DirectionalLight>(gfx, lightIdx++, true, true, pLightModelAsset, 30.f, 30.f, dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 50.0f, 5.0f);
		//pLights.emplace_back(pMainLight);

		pLightInputCB = std::make_unique<ConstantBuffer<LightInputCB>>(gfx, D3D11_USAGE_DYNAMIC);
		pClusteredIndices = std::make_unique<StructuredBuffer<int>>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, GetClusterCount() * MaxLightsPerCluster);

		// Create grid of lights
		const bool renderLightGrid = false;
		for (int x = -3; x <= 3; ++x)
		{
			for (int y = -3; y <= 3; ++y)
			{
				pLights.emplace_back(std::make_shared<PointLight>(gfx, lightIdx++, renderLightGrid, false, pLightModelAsset, dx::XMFLOAT3(x * 2.f, 1.5f, y * 2.f), RandomSaturatedColorRGB(), 3.f, 3.f, 5.f));
			}
		}

		/*int shadowLightCt = 0;
		for (const auto& l : pLights)
		{
			if (l->HasShadow())
			{
				shadowLightCt++;
			}
		}*/
		pLightShadowSB = std::make_unique<StructuredBuffer<LightShadowData>>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, Config::ShadowAtlasTileCount);
		cachedShadowData.resize(Config::ShadowAtlasTileCount);

		pShadowRendererList = std::make_shared<RendererList>(pRendererList);

		pShadowAtlas = std::make_unique<DepthStencilTarget>(gfx, Config::ShadowAtlasResolution, Config::ShadowAtlasResolution);

		// Setup cluster dimensions based on screen size, rounding up
		clusterDimensionX = ((UINT)gfx.GetScreenWidth() + LightManager::ClusteredLightingClusterPixels - 1u) / LightManager::ClusteredLightingClusterPixels;
		clusterDimensionY = ((UINT)gfx.GetScreenHeight() + LightManager::ClusteredLightingClusterPixels - 1u) / LightManager::ClusteredLightingClusterPixels;
		clusterDimensionZ = ClusteredLightingZLevels;
	}

	void LightManager::AddLightModelsToList(RendererList & pRendererList)
	{
		for (const auto& l : pLights)
		{
			if (l->GetModelInstance() != nullptr)
			{
				pRendererList.AddModelInstance(*l->GetModelInstance());
			}
		}
	}

	void LightManager::CullLights(Graphics & gfx, const std::unique_ptr<Camera>& cam, bool enableShadows)
	{
		dx::XMFLOAT4 frustumCornersVS;
		dx::XMStoreFloat4(&frustumCornersVS, cam->GetFrustumCornersVS());

		dx::XMVECTOR aabbCenter = dx::XMVectorSet(0.f, 0.f, (cam->GetNearClipPlane() + cam->GetFarClipPlane()) * 0.5f, 0.f);
		dx::XMVECTOR aabbExtents = dx::XMVectorSet(frustumCornersVS.z * 0.5f, frustumCornersVS.w * -0.5f, cam->GetFarClipPlane() - cam->GetNearClipPlane(), 0.f);

		visibleLightCt = 0u;
		int shadowMapIdx = 0;
		for (int i = 0; i < pLights.size(); ++i)
		{
			if (enableShadows && pLights[i]->HasShadow())
			{
				pLights[i]->SetCurrentShadowIdx(shadowMapIdx);
				shadowMapIdx += pLights[i]->GetShadowTileCount();
			}

			const auto data = pLights[i]->GetLightData(cam->GetViewMatrix());

			bool inFrustum = true;
			switch (pLights[i]->GetLightType())
			{
			case 0:
				// Point light
				inFrustum &= FrustumSphereIntersection(data.positionVS_range, frustumCornersVS, cam->GetFarClipPlane());
				break;
			case 1:
				// Spotlight (move sphere to middle and half the size)
				dx::XMFLOAT4 positionRange;
				dx::XMStoreFloat4(&positionRange, data.positionVS_range);
				auto lightSphere = dx::XMVectorAdd(data.positionVS_range, dx::XMVectorScale(data.directionVS, positionRange.w * 0.5f));
				lightSphere = dx::XMVectorSetW(lightSphere, positionRange.w * 0.5f);
				inFrustum &= FrustumSphereIntersection(lightSphere, frustumCornersVS, cam->GetFarClipPlane());
				break;
			case 2:
				// Directional
				break;
			}

			if (inFrustum)
			{
				cachedLightData[visibleLightCt++] = data;
			}
		}
		// Update SB
		pLightData->Update(gfx, cachedLightData, visibleLightCt);

		// Update CB
		LightInputCB lightInputCB;
		ZeroMemory(&lightInputCB, sizeof(lightInputCB));
		lightInputCB.visibleLightCount = visibleLightCt;
		lightInputCB.shadowAtlasTexelResolution = dx::XMVectorSet((float)Config::ShadowAtlasResolution, (float)Config::ShadowAtlasResolution, 1.f / (float)Config::ShadowAtlasResolution, 1.f / (float)Config::ShadowAtlasResolution);

		if (pMainLight)
		{
			lightInputCB.shadowCascadeSphere0 = pMainLight->GetShadowCascadeSphereVS(0);
			lightInputCB.shadowCascadeSphere1 = pMainLight->GetShadowCascadeSphereVS(1);
			lightInputCB.shadowCascadeSphere2 = pMainLight->GetShadowCascadeSphereVS(2);
			lightInputCB.shadowCascadeSphere3 = pMainLight->GetShadowCascadeSphereVS(3);
		}

		pLightInputCB->Update(gfx, lightInputCB);
		gfx.GetContext()->CSSetConstantBuffers(RenderSlots::CS_LightInputCB, 1u, pLightInputCB->GetD3DBuffer().GetAddressOf());
		gfx.GetContext()->CSSetConstantBuffers(RenderSlots::PS_LightInputCB, 1u, pLightInputCB->GetD3DBuffer().GetAddressOf());
	}

	std::unique_ptr<DepthStencilTarget>& LightManager::GetShadowAtlas()
	{
		return pShadowAtlas;
	}

	std::unique_ptr<ConstantBuffer<LightInputCB>>& LightManager::GetLightInputCB()
	{
		return pLightInputCB;
	}

	void LightManager::RenderShadows(ShadowPassContext context)
	{
		context.pRendererList = pShadowRendererList;

		pShadowAtlas->Clear(context.gfx);
		context.gfx.GetContext()->OMSetRenderTargets(0, nullptr, pShadowAtlas->GetView().Get());

		// todo: cull shadows
		for (int i = 0; i < pLights.size(); ++i)
		{
			if (pLights[i]->HasShadow())
			{
				int shadowMapIdx = pLights[i]->GetCurrentShadowIdx();
				pLights[i]->RenderShadow(context); //, gfx, cam, pass, pTransformationCB
				pLights[i]->AppendShadowData(shadowMapIdx, cachedShadowData);
				shadowMapIdx += pLights[i]->GetShadowTileCount();
			}
		}

		pLightShadowSB->Update(context.gfx, cachedShadowData, (UINT)cachedShadowData.size());
	}

	ComPtr<ID3D11ShaderResourceView> LightManager::GetLightDataSRV() const
	{
		return pLightData->GetSRV();
	}

	ComPtr<ID3D11ShaderResourceView> LightManager::GetShadowDataSRV() const
	{
		return pLightShadowSB->GetSRV();
	}

	void LightManager::DrawImguiControlWindows()
	{
		for (auto& l : pLights)
		{
			l->DrawImguiControlWindow();
		}
	}

	UINT LightManager::GetLightCount() const
	{
		return (UINT)pLights.size();
	}

	std::shared_ptr<Light> LightManager::GetLight(UINT index) const
	{
		return pLights[index];
	}

	UINT LightManager::GetClusterCount() const
	{
		return clusterDimensionX * clusterDimensionY * clusterDimensionZ;
	}

	UINT LightManager::GetClusterDimensionX() const
	{
		return clusterDimensionX;
	}

	UINT LightManager::GetClusterDimensionY() const
	{
		return clusterDimensionY;
	}

	UINT LightManager::GetClusterDimensionZ() const
	{
		return clusterDimensionZ;
	}

	const std::unique_ptr<StructuredBuffer<int>>& LightManager::GetClusteredIndices() const
	{
		return pClusteredIndices;
	}

	bool LightManager::FrustumSphereIntersection(dx::XMVECTOR lightSphere, dx::XMFLOAT4 frustumCorners, float farClipPlane)
	{
		// Use cross product to turn tile view directions into plane directions
		// The cross product is done by flipping X or Y with Z
		// Also, flip y here as it was previously flipped for GPU
		const auto plane0 = dx::XMVector3Normalize(dx::XMVectorSet(1, 0, -frustumCorners.x, 0));
		const auto plane1 = dx::XMVector3Normalize(dx::XMVectorSet(0, 1, frustumCorners.y, 0));
		const auto plane2 = dx::XMVector3Normalize(dx::XMVectorSet(-1, 0, -frustumCorners.x, 0));
		const auto plane3 = dx::XMVector3Normalize(dx::XMVectorSet(0, -1, frustumCorners.y, 0));

		dx::XMFLOAT4 positionVS_range;
		dx::XMStoreFloat4(&positionVS_range, lightSphere);

		bool inFrustum = true;
		float d;
		dx::XMStoreFloat(&d, dx::XMVector3Dot(lightSphere, plane0));
		inFrustum &= d <= positionVS_range.w;
		dx::XMStoreFloat(&d, dx::XMVector3Dot(lightSphere, plane1));
		inFrustum &= d <= positionVS_range.w;
		dx::XMStoreFloat(&d, dx::XMVector3Dot(lightSphere, plane2));
		inFrustum &= d <= positionVS_range.w;
		dx::XMStoreFloat(&d, dx::XMVector3Dot(lightSphere, plane3));
		inFrustum &= d <= positionVS_range.w;

		inFrustum &= positionVS_range.z <= farClipPlane;

		return inFrustum;
	}

	bool LightManager::AABBSphereIntersection(const LightData & lightData, dx::XMVECTOR aabbCenter, dx::XMVECTOR aabbExtents)
	{
		dx::XMFLOAT4 positionVS_range;
		dx::XMStoreFloat4(&positionVS_range, lightData.positionVS_range);

		// Formula: float3 displ = max(0, abs(aabbCenter - spherePos) - aabbExtents);
		dx::XMVECTOR displ = dx::XMVectorAbs(dx::XMVectorSubtract(aabbCenter, dx::XMVectorSet(positionVS_range.x, positionVS_range.y, positionVS_range.z, 0.f)));
		displ = dx::XMVectorSubtract(displ, aabbExtents);
		displ = dx::XMVectorMax(dx::XMVectorSet(0, 0, 0, 0), displ);

		float sdfSqr;
		dx::XMStoreFloat(&sdfSqr, dx::XMVector3Dot(displ, displ));

		return sdfSqr <= positionVS_range.w * positionVS_range.w;
	}
}