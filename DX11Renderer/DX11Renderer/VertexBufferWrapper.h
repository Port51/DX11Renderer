#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"

class Graphics;
class BaseBufferData;
struct ID3D11Buffer;

class VertexBufferWrapper
{
public:
	VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer);
	VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const;
	void BindIA(Graphics& gfx, UINT slot);
	size_t GetVertexCount() const;
protected:
	void SetupVertexBuffer(Graphics& gfx, const BaseBufferData& data);
	void SetupInstanceBuffer(Graphics& gfx, const BaseBufferData& data);
protected:
	size_t vertexCount;
	std::vector<UINT> strides;
	std::vector<UINT> offsets;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> pBufferArray;
};
