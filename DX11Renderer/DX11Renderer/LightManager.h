#pragma once
#include "StructuredBuffer.h"
#include "Renderer.h"
#include "RenderConstants.h"
#include "PointLight.h"
#include "Spotlight.h"
#include "DirectionalLight.h"
#include "RenderTarget.h"

struct LightData;

class LightManager
{
private:
	const static int MaxLightCount = 256;
public:
	LightManager(Graphics& gfx)
	{
		pLightData = std::make_unique<StructuredBuffer<LightData>>(gfx, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, MaxLightCount);
		cachedLightData.resize(MaxLightCount);

		for (int i = 0; i < 3; ++i)
		{
			pLights.emplace_back(std::make_unique<PointLight>(gfx, i, dx::XMFLOAT3((i - 1) * 7.5, 1.0f, 0.f), dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 3.0f));
			//pLights.emplace_back(std::make_unique<PointLight>(gfx, dx::XMFLOAT3(4.2f + i, 4.2f, -5.3f), dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 9.0f));
		}
		pLights.emplace_back(std::make_unique<Spotlight>(gfx, 3, dx::XMFLOAT3(0.0, 1.0f, -5.f), 0.0f, 0.0f, dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 50.0f, 5.0f));
		pLights.emplace_back(std::make_unique<DirectionalLight>(gfx, 4, dx::XMFLOAT3(-2.0, 1.0f, -5.f), 0.0f, 0.0f, dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 50.0f, 5.0f));

		pLightInputCB = std::make_unique<ConstantBuffer<LightInputCB>>(gfx, D3D11_USAGE_DYNAMIC);
	}
public:
	void SubmitDrawCalls(std::unique_ptr<Renderer>& fc) const
	{
		for (const auto& l : pLights)
		{
			l->SubmitDrawCalls(fc);
		}
	}
	void CullLights(Graphics& gfx, const Camera& cam)
	{
		dx::XMFLOAT4 frustumCornersVS;
		dx::XMStoreFloat4(&frustumCornersVS, cam.GetFrustumCornersVS());

		dx::XMVECTOR aabbCenter = dx::XMVectorSet(0.f, 0.f, (cam.GetNearClipPlane() + cam.GetFarClipPlane()) * 0.5f, 0.f);
		dx::XMVECTOR aabbExtents = dx::XMVectorSet(frustumCornersVS.z * 0.5f, frustumCornersVS.w * -0.5f, cam.GetFarClipPlane() - cam.GetNearClipPlane(), 0.f);

		visibleLightCt = 0u;
		for (int i = 0; i < pLights.size(); ++i)
		{
			const auto data = pLights[i]->GetLightData(cam.GetViewMatrix());

			bool inFrustum = true;
			switch (pLights[i]->GetLightType())
			{
			case 0:
				// Point light
				inFrustum &= FrustumSphereIntersection(data.positionVS_range, frustumCornersVS, cam.GetFarClipPlane());
				break;
			case 1:
				// Spotlight (move sphere to middle and half the size)
				dx::XMFLOAT4 positionRange;
				dx::XMStoreFloat4(&positionRange, data.positionVS_range);
				auto lightSphere = dx::XMVectorAdd(data.positionVS_range, dx::XMVectorScale(data.direction, positionRange.w * 0.5f));
				lightSphere = dx::XMVectorSetW(lightSphere, positionRange.w * 0.5f);
				inFrustum &= FrustumSphereIntersection(lightSphere, frustumCornersVS, cam.GetFarClipPlane());
				break;
			case 2:
				// Directional (do nothing)
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
		pLightInputCB->Update(gfx, lightInputCB);
		gfx.GetContext()->CSSetConstantBuffers(RenderSlots::CS_LightInputCB, 1u, pLightInputCB->GetD3DBuffer().GetAddressOf());
	}
	void RenderShadows(Graphics& gfx, const Camera& cam)
	{
		// todo: cull shadows
		for (int i = 0; i < pLights.size(); ++i)
		{
			if (pLights[i]->HasShadow())
			{
				pLights[i]->RenderShadow(gfx, cam);
			}
		}
	}
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetD3DSRV() const
	{
		return pLightData->GetD3DSRV();
	}
	void Bind(Graphics& gfx, UINT slot)
	{
		pLightData->BindCS(gfx, slot);
	}
	void DrawImguiControlWindows()
	{
		for (auto& l : pLights)
		{
			l->DrawImguiControlWindow();
		}
	}
private:
	bool FrustumSphereIntersection(dx::XMVECTOR lightSphere, dx::XMFLOAT4 frustumCorners, float farClipPlane)
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
	bool AABBSphereIntersection(const LightData& lightData, dx::XMVECTOR aabbCenter, dx::XMVECTOR aabbExtents)
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
private:
	std::vector<std::shared_ptr<Light>> pLights;
	std::vector<LightData> cachedLightData;
	UINT visibleLightCt;
private:
	std::unique_ptr<StructuredBuffer<LightData>> pLightData;
	std::unique_ptr<ConstantBuffer<LightInputCB>> pLightInputCB;
};