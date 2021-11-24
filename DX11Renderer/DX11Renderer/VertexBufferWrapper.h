#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"

class Graphics;
class BaseBufferData;
struct ID3D11Buffer;

class VertexBufferWrapper
{
public:
	VertexBufferWrapper(Graphics& gfx, const std::string& tag, const BaseBufferData& vbuf);
	VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vbuf);
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const;
	void BindIA(Graphics& gfx, UINT slot);
	UINT GetVertexCount() const;
	UINT GetStride() const;
protected:
	void SetupVertexBuffer(Graphics& gfx, const BaseBufferData& data);
	void SetupInstanceBuffer(Graphics& gfx, const BaseBufferData& data);
protected:
	std::string tag;
	UINT strides;
	UINT vertexCount;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> pBufferArray;
};
