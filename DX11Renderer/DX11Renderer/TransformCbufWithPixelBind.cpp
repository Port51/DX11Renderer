#include "TransformCbufWithPixelBind.h"
/*#include "Drawable.h"
#include "Graphics.h"

TransformCbufWithPixelBind::TransformCbufWithPixelBind(Graphics& gfx, const Drawable& parent, UINT slotV, UINT slotP)
	: TransformCbuf(gfx, parent, slotV)
{
	if (!pPcbuf)
	{
		pPcbuf = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, "NotInCodex", slotP);
	}
}

void TransformCbufWithPixelBind::Bind(Graphics& gfx)
{
	const auto tf = GetTransforms(gfx);
	UpdateBindImpl(gfx, tf);
}

void TransformCbufWithPixelBind::UpdateBindImpl(Graphics& gfx, const Transforms& transforms)
{
	TransformCbuf::UpdateBindImpl(gfx, transforms);
	pPcbuf->Update(gfx, transforms);
	pPcbuf->Bind(gfx);
}

std::unique_ptr<PixelConstantBuffer<TransformCbuf::Transforms>> TransformCbufWithPixelBind::pPcbuf;
*/