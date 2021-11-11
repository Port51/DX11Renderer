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

MeshRenderer::MeshRenderer(Graphics& gfx, std::string name, std::shared_ptr<Material> pMaterial,
	std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<IndexBuffer> pIndexBuffer, std::shared_ptr<Topology> pTopologyBuffer)
	: Drawable(pVertexBuffer, pIndexBuffer, pTopologyBuffer),
	name(name),
	pMaterial(pMaterial)
{
	assert("Material cannot be null" && pMaterial);

	/*if (!hasIndexBuffer)
	{
		throw std::runtime_error(std::string("Mesh '") + name + std::string("' is missing IndexBuffer!"));
	}*/

	//AddBind(std::make_shared<TransformCbuf>(gfx, *this, 0u));
}

DirectX::XMMATRIX MeshRenderer::GetTransformXM() const
{
	return DirectX::XMLoadFloat4x4(&transform);

	//return DirectX::XMMatrixRotationRollPitchYaw(roll, pitch, yaw) *
	//	DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void MeshRenderer::SubmitDrawCalls(FrameCommander & frame, dx::FXMMATRIX _accumulatedTranform) const
{
	dx::XMStoreFloat4x4(&transform, _accumulatedTranform);
	Drawable::SubmitDrawCalls(frame);
}
