#include "TransformCbuf.h"
#include "MeshRenderer.h"
#include "Graphics.h"
#include "ConstantBuffer.h"
#include "Transforms.h"

TransformCbuf::TransformCbuf(Graphics& gfx, const MeshRenderer& parent)
	: pParent(&parent)
{
	if (!pVcbuf)
	{
		pVcbuf = std::make_unique<ConstantBuffer<Transforms>>(gfx, D3D11_USAGE_DYNAMIC);
	}
}

void TransformCbuf::BindVS(Graphics& gfx, UINT slot)
{
	// debug:
	const auto modelMatrix = pParent->GetTransformXM();
	const auto modelViewMatrix = modelMatrix * gfx.GetViewMatrix();
	const auto modelViewProjectMatrix = modelViewMatrix * gfx.GetProjectionMatrix();
	Transforms transforms{ modelMatrix, modelViewMatrix, modelViewProjectMatrix };
	UpdateBindImpl(gfx, transforms);

	gfx.GetContext()->VSSetConstantBuffers(slot, 1u, pVcbuf->GetD3DBuffer().GetAddressOf());
}

void TransformCbuf::InitializeParentReference(const MeshRenderer& parent)
{
	auto newParent = &parent;
	pParent = &parent;
}

void TransformCbuf::UpdateBindImpl(Graphics& gfx, const Transforms& transforms)
{
	pVcbuf->Update(gfx, transforms);
}

// Because static
std::unique_ptr<ConstantBuffer<Transforms>> TransformCbuf::pVcbuf;