#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include <vector>

class Graphics;
class BaseBufferData;

struct ID3D11Buffer;

class VertexBufferWrapper
{
public:
	VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer);
	VertexBufferWrapper(Graphics& gfx, const BaseBufferData& vertexBuffer, const BaseBufferData& instanceBuffer);
public:
	ComPtr<ID3D11Buffer> GetVertexBuffer() const;
	void BindIA(Graphics& gfx, UINT slot);
	UINT GetVertexCount() const;
protected:
	void SetupVertexBuffer(Graphics& gfx, const BaseBufferData& data);
	void SetupInstanceBuffer(Graphics& gfx, const BaseBufferData& data);
protected:
	UINT vertexCount;
	std::vector<UINT> strides;
	std::vector<UINT> offsets;
	std::vector<ComPtr<ID3D11Buffer>> pBufferArray;
};
