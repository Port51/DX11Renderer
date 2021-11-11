#include "Drawable.h"
#include "GraphicsThrowMacros.h"
#include "BindableInclude.h"
#include "BindableCodex.h"

void Drawable::SubmitDrawCalls(FrameCommander& frame) const noexcept
{
	// todo: use material instead
	for (const auto& tech : techniques)
	{
		tech.SubmitDrawCalls(frame, *this);
	}
}

Drawable::Drawable(std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
	: pVertices(std::move(pVertexBuffer)),
	pIndices(std::move(pIndexBuffer)),
	pTopology(std::move(pTopologyBuffer))
{
}

void Drawable::AddTechnique(Technique tech_in) noexcept
{
	tech_in.InitializeParentReferences(*this);
	techniques.push_back(std::move(tech_in));
}

void Drawable::Bind(Graphics& gfx) const noexcept
{
	pTopology->Bind(gfx);
	pIndices->Bind(gfx);
	pVertices->Bind(gfx);
}

UINT Drawable::GetIndexCount() const
{
	return pIndices->GetCount();
}

Drawable::~Drawable()
{}