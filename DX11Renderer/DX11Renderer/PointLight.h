#pragma once
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "PixelConstantBuffer.h"
#include "ModelInstance.h"

class PointLight
{
public:
	PointLight(Graphics& gfx, DirectX::XMFLOAT3 positionWS, DirectX::XMFLOAT3 color, float intensity = 1.f, float range = 2.5f);
	void DrawImguiControlWindow();
	void Draw(Graphics& gfx) const;
	void Bind(Graphics& gfx, DirectX::FXMMATRIX viewMatrix) const;
private:
	struct PointLightCBuf
	{
		//alignas(16) 
		DirectX::XMFLOAT3 positionVS;
		float range;
		DirectX::XMFLOAT3 color;
		float intensity;
	};
private:
	PointLightCBuf cbData;
	DirectX::XMFLOAT3 positionWS;
	DirectX::XMFLOAT3 color;
	float intensity;
	float range;

	std::unique_ptr<ModelInstance> pModel;
	mutable PixelConstantBuffer<PointLightCBuf> globalLightCbuf;
};