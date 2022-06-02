#include "pch.h"
#include "MaterialPass.h"
#include "MeshRenderer.h"
#include "Binding.h"
#include "DrawContext.h"
#include "DrawCall.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "DrawContext.h"
#include "Renderer.h"
#include <assert.h>

namespace gfx
{
	MaterialPass::MaterialPass()
	{

	}

	void MaterialPass::Release()
	{
		
	}

	void MaterialPass::SetPixelShader(std::shared_ptr<PixelShader> pPixelShader)
	{
		m_pPixelShader = pPixelShader;
	}

	void MaterialPass::SetVertexShader(std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<InputLayout> pInputLayout)
	{
		assert(pVertexShader != nullptr);
		assert(pInputLayout != nullptr);
		m_pVertexShader = pVertexShader;
		m_pInputLayout = pInputLayout;
	}

	/*void MaterialPass::SetVertexLayout(std::shared_ptr<VertexLayout> pVertexLayout)
	{
		m_pVertexLayout = pVertexLayout;
	}*/

	Binding & MaterialPass::AddBinding(std::shared_ptr<Bindable> pBindable)
	{
		assert(pBindable != nullptr);
		m_bindings.push_back(Binding(std::move(pBindable)));
		return m_bindings[m_bindings.size() - 1];
	}

	const u64 MaterialPass::GetMaterialCode() const
	{
		// Create sorting code that minimizes state changes
		// Cost determines the order:
		// Shader program > ROP > texture bindings > vertex format > UBO bindings > vert bindings > uniform updates

		// Actually use 48 bits for this, as 16 will be used for depth
		// 10 bits - pixel shader (1024 possible)
		// 10 bits - vertex shader (1024 possible)
		// 2 bits - rasterizer state (4 possible)
		// 12 bits - texture bindings
		// 8 bits - vertex layout (256 possible)
		// 6 bits - UBO bindings (64 possible)
		auto ps = (m_pPixelShader != nullptr) ? m_pPixelShader->GetInstanceIdx() : 0u;
		auto vs = (m_pVertexShader != nullptr) ? m_pVertexShader->GetInstanceIdx() : 0u;
		return ps << 38u
			+ vs << 28u;
	}

	int MaterialPass::GetPropertySlot() const
	{
		return m_propertySlotIdx;
	}

	void MaterialPass::SetPropertySlot(const int slotIdx)
	{
		m_propertySlotIdx = slotIdx;
	}

	void MaterialPass::SetRenderPass(const RenderPassType _renderPass)
	{
		assert(_renderPass != RenderPassType::Undefined);
		m_renderPass = _renderPass;
	}

	const RenderPassType MaterialPass::GetRenderPass() const
	{
		return m_renderPass;
	}

	void MaterialPass::SubmitDrawCommands(const MeshRenderer& meshRenderer, const DrawContext& drawContext, const BindingList* const pPropertyBindings) const
	{
		drawContext.renderer.AcceptDrawCall(std::move(DrawCall(this, &meshRenderer, drawContext, pPropertyBindings)), m_renderPass);
	}

	void MaterialPass::Bind(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		m_pInputLayout->BindIA(gfx, renderState, 0u);
		m_pVertexShader->BindVS(gfx, renderState, 0u);
		if (m_pPixelShader != nullptr)
		{
			m_pPixelShader->BindPS(gfx, renderState, 0u);
		}
		for (const auto& b : m_bindings)
		{
			b.Bind(gfx, renderState);
		}
	}

}