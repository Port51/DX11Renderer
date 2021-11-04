#include "Drawable.h"
#include "GraphicsThrowMacros.h"
#include "IndexBuffer.h"
#include <cassert>

void Drawable::Draw(Graphics& gfx) const
{
	// Bind all resources
	for (auto& b : binds) // instance binds
	{
		b->Bind(gfx);
	}

	// Issue draw call
	gfx.DrawIndexed(pIndexBuffer->GetCount());
}

void Drawable::AddBind(std::shared_ptr<Bindable> bind)
{
	if (typeid(*bind) == typeid(IndexBuffer))
	{
		assert("Cannot bind multiple index buffers" && pIndexBuffer == nullptr);
		pIndexBuffer = &static_cast<IndexBuffer&>(*bind);
	}
	binds.push_back(std::move(bind));
}