#include "pch.h"
#include "Bindable.h"

namespace gfx
{
	void Bindable::BindIA(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("BindIA() not implemented on type derived from Bindable");
	}

	void Bindable::BindCS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("BindCS() not implemented on type derived from Bindable");
	}

	void Bindable::BindVS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("BindVS() not implemented on type derived from Bindable");
	}

	void Bindable::BindPS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("BindPS() not implemented on type derived from Bindable");
	}

	void Bindable::BindRS(const GraphicsDevice & gfx, RenderState & renderState)
	{
		THROW("BindRS() not implemented on type derived from Bindable");
	}

	void Bindable::BindOM(const GraphicsDevice & gfx, RenderState & renderState)
	{
		THROW("BindOM() not implemented on type derived from Bindable");
	}

	void Bindable::UnbindIA(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("UnbindIA() not implemented on type derived from Bindable");
	}

	void Bindable::UnbindCS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("UnbindCS() not implemented on type derived from Bindable");
	}

	void Bindable::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("UnbindVS() not implemented on type derived from Bindable");
	}

	void Bindable::UnbindPS(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		THROW("UnbindPS() not implemented on type derived from Bindable");
	}

	void Bindable::UnbindRS(const GraphicsDevice & gfx, RenderState & renderState)
	{
		THROW("UnbindRS() not implemented on type derived from Bindable");
	}

	void Bindable::UnbindOM(const GraphicsDevice & gfx, RenderState & renderState)
	{
		THROW("UnbindOM() not implemented on type derived from Bindable");
	}
}
