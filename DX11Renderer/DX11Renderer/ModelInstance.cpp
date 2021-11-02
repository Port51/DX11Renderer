#include "ModelInstance.h"
#include "BindableInclude.h"
#include "GraphicsThrowMacros.h"
#include "PixelConstantBuffer.h"
#include "FBXLoader.h"
#include "VertexInclude.h"
//#include "Sphere.h"

namespace dx = DirectX;

ModelInstance::ModelInstance(Graphics& gfx, DirectX::XMFLOAT3 materialColor, dx::XMFLOAT3 instanceScale)
{

	if (!IsStaticInitialized())
	{
		//using hw3dexp::VertexLayout;
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 normal;
		};
		VertexBufferData vbuf(std::move(
			VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
		));

		//auto model = Sphere::Make<Vertex>();
		IndexedTriangleList<Vertex> model;
		//FBXLoader::LoadFBX("Models\\CubeFBX.fbx", &model);
		FBXLoader::LoadFBX("Models\\HeadTriangulated.fbx", &model, 1.5f);
		model.SetNormalsIndependentFlat();

		for (unsigned int i = 0; i < model.vertices.size(); ++i)
		{
			vbuf.EmplaceBack(
				model.vertices[i].pos,
				model.vertices[i].normal
			);
		}

		//model.Transform(dx::XMMatrixScaling(1.f, 1.f, 1.2f));
		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vbuf));

		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, model.indices));

		/*const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
			// IN ORDER:
			// Semantic "Position" must match vertex shader semantic
			// The 0 after = index of semantic
			// Format = simple...
			// Slot = ...
			// Offset in bytes in structure
			// Vert vs. instances
			// Instance stuff
			{ "Position", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "Color", 0, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};*/
		AddStaticBind(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		SetIndexFromStatic();
	}

	// Instance bind
	AddBind(std::make_unique<TransformCbuf>(gfx, *this));

	struct PSMaterialConstant
	{
		dx::XMFLOAT3 color;
		float specularIntensity = 0.6f;
		float specularPower = 30.0f;
		float padding[3];
	} colorConst;
	colorConst.color = materialColor;
	AddBind(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, colorConst, 1u));

	// Instance scaling
	dx::XMStoreFloat3x3(&mt, dx::XMMatrixScaling(instanceScale.x, instanceScale.y, instanceScale.z));
}

void ModelInstance::Update(float dt)
{
	// nothing yet
}

DirectX::XMMATRIX ModelInstance::GetTransformXM() const
{
	return DirectX::XMLoadFloat3x3(&mt);
}