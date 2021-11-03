#include "Mesh.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "PixelConstantBuffer.h"
#include "FBXLoader.h"
#include "VertexInclude.h"
#include <exception>
//#include "Sphere.h"

namespace dx = DirectX;

Mesh::Mesh(Graphics& gfx, std::string name, std::vector<std::unique_ptr<Bindable>> pBindables)
	: name(name)
{
	if (!IsStaticInitialized())
	{
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		//SetIndexFromStatic();
	}

	bool hasIndexBuffer = false;
	for (auto& pb : pBindables)
	{
		if (auto pi = dynamic_cast<IndexBuffer*>(pb.get()))
		{
			// Special treatment for index buffer as we need a ptr to that
			AddIndexBuffer(std::unique_ptr<IndexBuffer>{ pi });
			pb.release();
			hasIndexBuffer = true;
		}
		else
		{
			AddBind(std::move(pb));
		}
	}
	if (!hasIndexBuffer)
	{
		throw std::runtime_error(std::string("Mesh '") + name + std::string("' is missing IndexBuffer!"));
	}

	// Instance bind
	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
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