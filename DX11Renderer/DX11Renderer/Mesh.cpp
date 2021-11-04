#include "Mesh.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "PixelConstantBuffer.h"
#include "FBXLoader.h"
#include "VertexInclude.h"
#include <exception>
//#include "Sphere.h"

namespace dx = DirectX;

Mesh::Mesh(Graphics& gfx, std::string name, std::vector<std::shared_ptr<Bindable>> pBindables)
	: name(name)
{
	AddBind(std::make_shared<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	bool hasIndexBuffer = false;
	for (auto& pb : pBindables)
	{
		AddBind(std::move(pb));
	}
	if (!hasIndexBuffer)
	{
		throw std::runtime_error(std::string("Mesh '") + name + std::string("' is missing IndexBuffer!"));
	}

	AddBind(std::make_shared<TransformCbuf>(gfx, *this));
}

void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noexcept(!IS_DEBUG)
{
	DirectX::XMStoreFloat4x4(&modelMatrix, accumulatedTransform);
	Drawable::Draw(gfx);
}

DirectX::XMMATRIX Mesh::GetTransformXM() const
{
	return DirectX::XMLoadFloat4x4(&modelMatrix);
}