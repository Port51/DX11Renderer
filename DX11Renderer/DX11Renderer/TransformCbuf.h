#pragma once
#include "VertexConstantBuffer.h"
#include "Drawable.h"
#include <DirectXMath.h>

class TransformCbuf : public Bindable
{
protected:
	struct Transforms
	{
		DirectX::XMMATRIX model;
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProj;
	};
public:
	TransformCbuf(Graphics& gfx, const Drawable& parent, UINT slot = 0u);
	void Bind(Graphics& gfx) override;
protected:
	virtual void UpdateBindImpl(Graphics& gfx, const Transforms& transforms);
	Transforms GetTransforms(Graphics& gfx);
private:
	// Static so can be re-used each drawcall
	static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
	const Drawable& parent;
};
