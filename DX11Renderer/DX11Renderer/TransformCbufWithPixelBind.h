#pragma once
#include "PixelConstantBuffer.h"
#include "TransformCbuf.h"
#include "WindowsInclude.h"

class Graphics;
class Drawable;

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