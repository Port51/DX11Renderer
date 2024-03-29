#include "pch.h"
#include "LightManager.h"

#include "GraphicsDevice.h"
#include "Camera.h"
#include "StructuredBuffer.h"
#include "ConstantBuffer.h"
#include "Renderer.h"
#include "RenderStats.h"
#include "RenderConstants.h"
#include "PointLight.h"
#include "Spotlight.h"
#include "DirectionalLight.h"
#include "RenderTexture.h"
#include "RenderPass.h"
#include "RendererList.h"
#include "Model.h"
#include "LightShadowData.h"
#include "DepthStencilTarget.h"
#include "ModelImporter.h"
#include "Config.h"
#include "ShadowPassContext.h"

namespace gfx
{
	LightManager::LightManager(const GraphicsDevice& gfx, const std::shared_ptr<RendererList>& pRendererList)
	{
		const auto pLightModelAsset = ModelImporter::LoadGLTF(gfx, "Assets\\Models\\DefaultSphere.asset");

		m_pLightData = std::make_unique<StructuredBuffer>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, MaxLightCount, sizeof(LightData));
		m_cachedLightData.resize(MaxLightCount);

		//UINT lightIdx = 0u;
		//m_pLights.emplace_back(std::make_shared<PointLight>(gfx, lightIdx++, true, true, pLightModelAsset, dx::XMFLOAT3(0.f, 5.4f, 0.f), dx::XMFLOAT3(1.f, 1.f, 1.f), 3.f, 3.f, 50.f));

		//pLights.emplace_back(std::make_shared<Spotlight>(gfx, lightIdx++, true, true, pLightModelAsset, dx::XMFLOAT3(8.5f, 1.5f, -2.0f), 0.0f, 0.0f, dx::XMFLOAT3(1.0f, 1.0f, 1.0f), 3.0f, 50.f, 50.f));
		//pMainLight = std::make_shared<DirectionalLight>(gfx, lightIdx++, true, true, pLightModelAsset, 30.f, 30.f, dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 50.0f, 5.0f);
		//pLights.emplace_back(pMainLight);

		// Setup cluster dimensions based on screen size, rounding up
		m_clusterDimensionX = (gfx.GetScreenWidth() + LightManager::ClusteredLightingClusterPixels - 1u) / LightManager::ClusteredLightingClusterPixels;
		m_clusterDimensionY = (gfx.GetScreenHeight() + LightManager::ClusteredLightingClusterPixels - 1u) / LightManager::ClusteredLightingClusterPixels;
		m_clusterDimensionZ = ClusteredLightingZLevels;

		m_pLightInputCB = std::make_unique<ConstantBuffer>(gfx, D3D11_USAGE_DYNAMIC, sizeof(LightInputCB));
		m_pClusteredIndices = std::make_unique<StructuredBuffer>(gfx, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, GetClusterCount() * MaxLightsPerCluster, sizeof(int));

		// Create grid of lights
		/*const bool renderLightGrid = false;
		for (int x = -3; x <= 3; ++x)
		{
			for (int y = -3; y <= 3; ++y)
			{
				m_pLights.emplace_back(std::make_shared<PointLight>(gfx, lightIdx++, renderLightGrid, false, pLightModelAsset, dx::XMFLOAT3(x * 2.f, 1.5f, y * 2.f), RandomSaturatedColorRGB(), 3.f, 3.f, 5.f));
			}
		}*/

		//m_lightVisibility.resize(m_pLights.size());

		/*int shadowLightCt = 0;
		for (const auto& l : pLights)
		{
			if (l->HasShadow())
			{
				shadowLightCt++;
			}
		}*/
		m_pLightShadowSB = std::make_unique<StructuredBuffer>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, Config::ShadowAtlasTileCount, sizeof(LightShadowData));
		m_cachedShadowData.resize(Config::ShadowAtlasTileCount);

		m_pShadowRendererList = std::make_shared<RendererList>(pRendererList);

		m_pShadowAtlas = std::make_unique<DepthStencilTarget>(gfx, Config::ShadowAtlasResolution, Config::ShadowAtlasResolution);
	}

	LightManager::~LightManager()
	{}

	void LightManager::Release()
	{
		m_pLightData->Release();
		m_pLightInputCB->Release();
		m_pLightShadowSB->Release();
		m_pShadowAtlas->Release();
		m_pClusteredIndices->Release();
	}

	void LightManager::AddLight(std::shared_ptr<Light> pLight)
	{
		m_pLights.emplace_back(std::move(pLight));
		m_lightVisibility.resize(m_pLights.size());
	}

	void LightManager::AddPointLight(const GraphicsDevice& gfx, const dx::XMVECTOR positionWS, const dx::XMFLOAT3 color, const float intensity, const float attenuationQ, const float range)
	{
		auto prevCount = m_pLights.size();

		const bool createDebugModels = false;
		if (createDebugModels)
		{
			const auto pLightModelAsset = ModelImporter::LoadGLTF(gfx, "Assets\\Models\\DefaultSphere.asset");
			AddLight(std::move(std::make_shared<PointLight>(gfx, prevCount, false, false, pLightModelAsset, positionWS, color, intensity, attenuationQ, range)));
		}
		else
		{
			AddLight(std::move(std::make_shared<PointLight>(gfx, prevCount, false, false, nullptr, positionWS, color, intensity, attenuationQ, range)));
		}
	}

	void LightManager::AddDirectionalLight(const GraphicsDevice& gfx, const float pan, const float tilt, const dx::XMFLOAT3 color, const float intensity)
	{
		auto prevCount = m_pLights.size();
		m_pMainLight = std::make_shared<DirectionalLight>(gfx, prevCount, false, true, nullptr, pan, tilt, color, intensity, 50.0f, 5.0f);
		AddLight(m_pMainLight);
	}

	void LightManager::AddLightModelsToList(RendererList& pRendererList)
	{
		for (const auto& l : m_pLights)
		{
			if (l->GetModel() != nullptr)
			{
				pRendererList.AddModel(*l->GetModel());
			}
		}
	}

	void LightManager::CullLightsAndShadows(const GraphicsDevice& gfx, const Camera& cam, const bool enableShadows)
	{
		dx::XMFLOAT4 frustumCornersVS;
		dx::XMStoreFloat4(&frustumCornersVS, cam.GetFrustumCornersVS());

		dx::XMVECTOR aabbCenter = dx::XMVectorSet(0.f, 0.f, (cam.GetNearClipPlane() + cam.GetFarClipPlane()) * 0.5f, 0.f);
		dx::XMVECTOR aabbExtents = dx::XMVectorSet(frustumCornersVS.z * 0.5f, frustumCornersVS.w * -0.5f, cam.GetFarClipPlane() - cam.GetNearClipPlane(), 0.f);

		m_visibleLightCt = 0u;
		int shadowMapIdx = 0;
		for (size_t i = 0; i < m_pLights.size(); ++i)
		{
			if (enableShadows && m_pLights[i]->HasShadow())
			{
				m_pLights[i]->SetCurrentShadowIdx(shadowMapIdx);
				shadowMapIdx += m_pLights[i]->GetShadowTileCount();
			}

			const auto data = m_pLights[i]->GetLightData(cam.GetViewMatrix());

			bool inFrustum = true;
			switch (m_pLights[i]->GetLightType())
			{
			case 0:
				// Point light
				inFrustum &= cam.GetFrustumVS().DoesSphereIntersect(data.positionVS_range);
				break;
			case 1:
				// Spotlight (move sphere to middle and half the size)
				dx::XMFLOAT4 positionRange;
				dx::XMStoreFloat4(&positionRange, data.positionVS_range);
				auto lightSphere = dx::XMVectorAdd(data.positionVS_range, dx::XMVectorScale(data.directionVS, positionRange.w * 0.5f));
				lightSphere = dx::XMVectorSetW(lightSphere, positionRange.w * 0.5f);
				inFrustum &= cam.GetFrustumVS().DoesSphereIntersect(lightSphere);
				break;
			case 2:
				// Directional - always render shadows
				break;
			}

			if (inFrustum)
			{
				m_cachedLightData[m_visibleLightCt++] = data;
				m_lightVisibility[i] = true;
				gfx.GetRenderStats().AddVisibleLights(1u);
			}
			else
			{
				m_lightVisibility[i] = false;
				gfx.GetRenderStats().AddCulledLights(1u);
			}
		}

		// Update SB
		m_pLightData->Update(gfx, m_cachedLightData.data(), m_visibleLightCt);

		// Update CB
		LightInputCB lightInputCB;
		ZERO_MEM(lightInputCB);
		lightInputCB.visibleLightCount = m_visibleLightCt;
		lightInputCB.shadowAtlasTexelResolution = dx::XMVectorSet((float)Config::ShadowAtlasResolution, (float)Config::ShadowAtlasResolution, 1.f / (float)Config::ShadowAtlasResolution, 1.f / (float)Config::ShadowAtlasResolution);

		if (m_pMainLight)
		{
			lightInputCB.shadowCascadeSphere0 = m_pMainLight->GetShadowCascadeSphereVS(0);
			lightInputCB.shadowCascadeSphere1 = m_pMainLight->GetShadowCascadeSphereVS(1);
			lightInputCB.shadowCascadeSphere2 = m_pMainLight->GetShadowCascadeSphereVS(2);
			lightInputCB.shadowCascadeSphere3 = m_pMainLight->GetShadowCascadeSphereVS(3);
		}

		m_pLightInputCB->Update(gfx, &lightInputCB);
		gfx.GetContext()->CSSetConstantBuffers(RenderSlots::CS_LightInputCB, 1u, m_pLightInputCB->GetD3DBuffer().GetAddressOf());
		gfx.GetContext()->CSSetConstantBuffers(RenderSlots::PS_LightInputCB, 1u, m_pLightInputCB->GetD3DBuffer().GetAddressOf());
	}

	DepthStencilTarget& LightManager::GetShadowAtlas()
	{
		return *m_pShadowAtlas.get();
	}

	ConstantBuffer& LightManager::GetLightInputCB()
	{
		return *m_pLightInputCB.get();
	}

	void LightManager::RenderShadows(ShadowPassContext& context, RenderState& renderState)
	{
		context.pRendererList = m_pShadowRendererList.get();

		m_pShadowAtlas->Clear(context.gfx);
		context.gfx.GetContext()->OMSetRenderTargets(0u, nullptr, m_pShadowAtlas->GetView().Get());
		REGISTER_GPU_CALL_GFX(context.gfx);

		// todo: cull shadows
		for (size_t i = 0; i < m_pLights.size(); ++i)
		{
			if (m_pLights[i]->HasShadow())
			{
				if (m_lightVisibility[i])
				{
					int shadowMapIdx = m_pLights[i]->GetCurrentShadowIdx();
					m_pLights[i]->RenderShadow(context, renderState);
					m_pLights[i]->AppendShadowData(shadowMapIdx, m_cachedShadowData);
					shadowMapIdx += m_pLights[i]->GetShadowTileCount();

					context.gfx.GetRenderStats().AddVisibleShadows(m_pLights[i]->GetShadowTileCount());
				}
				else
				{
					context.gfx.GetRenderStats().AddCulledShadows(m_pLights[i]->GetShadowTileCount());
				}
			}
		}

		m_pLightShadowSB->Update(context.gfx, m_cachedShadowData.data());
	}

	const ComPtr<ID3D11ShaderResourceView>& LightManager::GetLightDataSRV() const
	{
		return m_pLightData->GetSRV();
	}

	const ComPtr<ID3D11ShaderResourceView>& LightManager::GetShadowDataSRV() const
	{
		return m_pLightShadowSB->GetSRV();
	}

	void LightManager::DrawImguiControlWindows() const
	{
		for (auto& l : m_pLights)
		{
			l->DrawImguiControlWindow();
		}
	}

	const UINT LightManager::GetLightCount() const
	{
		return (UINT)m_pLights.size();
	}

	Light& LightManager::GetLight(const UINT index) const
	{
		return *m_pLights.at(index).get();
	}

	const UINT LightManager::GetClusterCount() const
	{
		return m_clusterDimensionX * m_clusterDimensionY * m_clusterDimensionZ;
	}

	const UINT LightManager::GetClusterDimensionX() const
	{
		return m_clusterDimensionX;
	}

	const UINT LightManager::GetClusterDimensionY() const
	{
		return m_clusterDimensionY;
	}

	const UINT LightManager::GetClusterDimensionZ() const
	{
		return m_clusterDimensionZ;
	}

	const StructuredBuffer& LightManager::GetClusteredIndices() const
	{
		return *m_pClusteredIndices.get();
	}

}