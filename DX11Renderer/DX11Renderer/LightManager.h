#pragma once
#include "StructuredBuffer.h"
#include "Renderer.h"
#include "RenderConstants.h"

class PointLight;
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
			pLights.emplace_back(std::make_unique<PointLight>(gfx, dx::XMFLOAT3(4.2f + i, 4.2f, -5.3f), dx::XMFLOAT3(1.f, 1.f, 1.f), 3.0, 9.0f));
		}

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
		visibleLightCt = 0u;
		for (int i = 0; i < pLights.size(); ++i)
		{
			// todo: cull light via frustum
			cachedLightData[visibleLightCt++] = pLights[i]->GetLightData(cam.GetViewMatrix());
		}
		pLightData->Update(gfx, cachedLightData, visibleLightCt);

		LightInputCB lightInputCB;
		ZeroMemory(&lightInputCB, sizeof(lightInputCB));
		lightInputCB.visibleLightCount = 3;
		pLightInputCB->Update(gfx, lightInputCB);
		gfx.GetContext()->CSSetConstantBuffers(RenderSlots::CS_LightInputCB, 1u, pLightInputCB->GetD3DBuffer().GetAddressOf());
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
	std::vector<std::shared_ptr<PointLight>> pLights;
	std::vector<LightData> cachedLightData;
	UINT visibleLightCt;
public:
	std::unique_ptr<StructuredBuffer<LightData>> pLightData;
	std::unique_ptr<ConstantBuffer<LightInputCB>> pLightInputCB;
};