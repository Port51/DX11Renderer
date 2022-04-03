#include "pch.h"
#include "TransformCbuf.h"
#include "MeshRenderer.h"
#include "GraphicsDevice.h"
#include "ConstantBuffer.h"
#include "Transforms.h"

namespace gfx
{
	TransformCbuf::TransformCbuf(GraphicsDevice& gfx)
	{
		if (!m_pVcbuf)
		{
			m_pVcbuf = std::make_unique<ConstantBuffer<Transforms>>(gfx, D3D11_USAGE_DYNAMIC);
		}
	}

	void TransformCbuf::BindVS(GraphicsDevice& gfx, UINT slot)
	{
		gfx.GetContext()->VSSetConstantBuffers(slot, 1u, m_pVcbuf->GetD3DBuffer().GetAddressOf());
	}

	void TransformCbuf::UpdateTransforms(GraphicsDevice& gfx, const Transforms& transforms)
	{
		m_pVcbuf->Update(gfx, transforms);
	}

	// Because static
	std::unique_ptr<ConstantBuffer<Transforms>> TransformCbuf::m_pVcbuf;
}