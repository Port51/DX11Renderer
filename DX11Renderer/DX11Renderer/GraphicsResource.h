#pragma once

class Graphics;
struct ID3D11DeviceContext;
struct ID3D11Device;
class DxgiInfoManager;

class GraphicsResource
{
protected:
	static ID3D11DeviceContext* GetContext(Graphics& gfx);
	static ID3D11Device* GetDevice(Graphics& gfx);
	static DxgiInfoManager& GetInfoManager(Graphics& gfx);
};