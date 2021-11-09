#include "MeshRenderer.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "PixelConstantBuffer.h"
#include "VertexInclude.h"
#include "TransformCbufWithPixelBind.h"
#include <exception>
#include <assert.h>
#include "Stencil.h"
//#include "Sphere.h"

namespace dx = DirectX;

MeshRenderer::MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial, std::vector<std::shared_ptr<Bindable>> pBindables)
	: name(name),
	pMaterial(pMaterial)
{
	assert("Material cannot be null" && pMaterial);
	AddBind(std::make_shared<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	if (name == "Sphere")
		AddBind(std::make_unique<Bind::Stencil>(gfx, Bind::Stencil::Mode::Mask));
	else
		AddBind(std::make_unique<Bind::Stencil>(gfx, Bind::Stencil::Mode::Write));

	for (auto& pb : pBindables)
	{
		AddBind(std::move(pb));
	}
	/*if (!hasIndexBuffer)
	{
		throw std::runtime_error(std::string("Mesh '") + name + std::string("' is missing IndexBuffer!"));
	}*/

	AddBind(std::make_shared<TransformCbuf>(gfx, *this, 0u));
}

void MeshRenderer::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noexcept(!IS_DEBUG)
{
	DirectX::XMStoreFloat4x4(&modelMatrix, accumulatedTransform);
	pMaterial->Bind(gfx);
	Drawable::Draw(gfx);
}

DirectX::XMMATRIX MeshRenderer::GetTransformXM() const
{
	return DirectX::XMLoadFloat4x4(&modelMatrix);
}