#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffer.h"
#include "PixelConstantBuffer.h"

class PointLight
{
public:
	PointLight(Graphics& gfx, DirectX::XMFLOAT3 position, float radius = 0.5f);
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
	DirectX::XMFLOAT3 initialPositionWS;
	PointLightCBuf cbData;
	mutable SolidSphere mesh;
	mutable PixelConstantBuffer<PointLightCBuf> cbuf;
};