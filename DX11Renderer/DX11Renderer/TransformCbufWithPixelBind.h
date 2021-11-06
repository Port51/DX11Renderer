#pragma once
#include "TransformCbuf.h"
#include "PixelConstantBuffer.h"

class TransformCbufWithPixelBind : public TransformCbuf
{
public:
	TransformCbufWithPixelBind(Graphics& gfx, const Drawable& parent, UINT slotV = 0u, UINT slotP = 0u);
	void Bind(Graphics& gfx) override;
protected:
	void UpdateBindImpl(Graphics& gfx, const Transforms& tf) override;
private:
	static std::unique_ptr<PixelConstantBuffer<Transforms>> pPcbuf;
};