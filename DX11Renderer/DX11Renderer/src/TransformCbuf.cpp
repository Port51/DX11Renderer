#include "pch.h"
#include "TransformCbuf.h"
#include "MeshRenderer.h"
#include "GraphicsDevice.h"
#include "ConstantBuffer.h"
#include "Transforms.h"
#include "CommonCbuffers.h"

namespace gfx
{
	TransformCbuf::TransformCbuf(const GraphicsDevice& gfx)
	{
		if (!m_pVcbuf)
		{
			m_pVcbuf = std::make_unique<ConstantBuffer>(gfx, D3D11_USAGE_DYNAMIC, sizeof(ObjectTransformsCB));
		}
	}

	void TransformCbuf::Release()
	{
		m_pVcbuf->Release();
	}

	void TransformCbuf::BindVS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::VS_CB, slot))
		{
			gfx.GetContext()->VSSetConstantBuffers(slot, 1u, m_pVcbuf->GetD3DBuffer().GetAddressOf());
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void TransformCbuf::UnbindVS(const GraphicsDevice & gfx, RenderState & renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::VS_CB, slot);
		gfx.GetContext()->VSSetConstantBuffers(slot, 1u, nullptr);
		REGISTER_GPU_CALL();
	}

	void TransformCbuf::BindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		if (renderState.IsNewBinding(GetGuid(), RenderBindingType::DS_CB, slot))
		{
			gfx.GetContext()->DSSetConstantBuffers(slot, 1u, m_pVcbuf->GetD3DBuffer().GetAddressOf());
			REGISTER_GPU_CALL();
		}
		else REGISTER_GPU_CALL_SAVED();
	}

	void TransformCbuf::UnbindDS(const GraphicsDevice& gfx, RenderState& renderState, const slotUINT slot)
	{
		renderState.ClearBinding(RenderBindingType::DS_CB, slot);
		gfx.GetContext()->DSSetConstantBuffers(slot, 1u, nullptr);
		REGISTER_GPU_CALL();
	}

	void TransformCbuf::UpdateTransforms(const GraphicsDevice& gfx, const ObjectTransformsCB& transforms)
	{
		m_pVcbuf->Update(gfx, &transforms);
	}

	// Because static
	std::unique_ptr<ConstantBuffer> TransformCbuf::m_pVcbuf;
}