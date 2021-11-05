#pragma once
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "PixelConstantBuffer.h"
#include "ModelInstance.h"

class PointLight
{
public:
	PointLight(Graphics& gfx, DirectX::XMFLOAT3 position, float intensity = 1.f, float radius = 0.5f);
	void DrawImguiControlWindow();
	void Reset();
	void Draw(Graphics& gfx) const;
	void Bind(Graphics& gfx, DirectX::FXMMATRIX viewMatrix) const;
private:
	struct PointLightCBuf
	{
		alignas(16) DirectX::XMFLOAT3 posVS;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	};
private:
	DirectX::XMFLOAT3 positionWS;
	std::unique_ptr<ModelInstance> pModel;
	DirectX::XMFLOAT3 initialPositionWS;
	float initialIntensity;
	PointLightCBuf cbData;
	mutable PixelConstantBuffer<PointLightCBuf> cbuf;
};