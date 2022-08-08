#include "pch.h"
#include "MaterialPass.h"
#include "Drawable.h"
#include "Binding.h"
#include "DrawContext.h"
#include "DrawCommand.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "DrawContext.h"
#include "Renderer.h"
#include "DepthStencilState.h"
#include <assert.h>

namespace gfx
{
	MaterialPass::MaterialPass()
	{

	}

	void MaterialPass::Release()
	{
		
	}

	void MaterialPass::SetInstanced(const bool isInstanced)
	{
		m_instanced = isInstanced;
	}

	void MaterialPass::SetPixelShader(std::shared_ptr<PixelShader> pPixelShader)
	{
		m_pPixelShader = std::move(pPixelShader);
	}

	void MaterialPass::SetVertexShader(std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<InputLayout> pInputLayout)
	{
		assert(pVertexShader != nullptr);
		assert(pInputLayout != nullptr);
		m_pVertexShader = std::move(pVertexShader);
		m_pInputLayout = std::move(pInputLayout);
	}

	void MaterialPass::SetHullShader(std::shared_ptr<HullShader> pHullShader)
	{
		m_pHullShader = std::move(pHullShader);
	}

	void MaterialPass::SetDomainShader(std::shared_ptr<DomainShader> pDomainShader)
	{
		m_pDomainShader = std::move(pDomainShader);
	}

	void MaterialPass::SetStencil(std::shared_ptr<DepthStencilState> pDepthStencilState)
	{
		m_pDepthStencilState = std::move(pDepthStencilState);
	}

	const std::shared_ptr<DepthStencilState> MaterialPass::GetStencil() const
	{
		return m_pDepthStencilState;
	}

	/*void MaterialPass::SetVertexLayout(std::shared_ptr<VertexLayout> pVertexLayout)
	{
		m_pVertexLayout = pVertexLayout;
	}*/

	void MaterialPass::AddShaderDefine(std::string define)
	{
		m_shaderDefines.emplace_back(define);
	}

	const std::vector<std::string>& MaterialPass::GetShaderDefines() const
	{
		return m_shaderDefines;
	}

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

		// Actually use 52 bits for this, as 12 will be used for depth
		// 4 bits - render queue
		// 10 bits - pixel shader (1024 possible)
		// 10 bits - vertex shader (1024 possible)
		// 2 bits - rasterizer state (4 possible)
		// 12 bits - texture bindings
		// 8 bits - vertex layout (256 possible)
		// 6 bits - UBO bindings (64 possible)
		auto ps = (m_pPixelShader != nullptr) ? m_pPixelShader->GetInstanceIdx() : 0u;
		auto vs = (m_pVertexShader != nullptr) ? m_pVertexShader->GetInstanceIdx() : 0u;
		return (static_cast<u64>(m_renderQueue) << 44u)
			+ (static_cast<u64>(ps) << 34u)
			+ (static_cast<u64>(vs) << 24u);
	}

	const int MaterialPass::GetPropertySlot() const
	{
		return m_propertySlotIdx;
	}

	void MaterialPass::SetPropertySlot(const int slotIdx)
	{
		m_propertySlotIdx = slotIdx;
	}

	const int MaterialPass::GetRenderQueue() const
	{
		return m_renderQueue;
	}

	void MaterialPass::SetRenderQueue(const u8 renderQueue)
	{
		m_renderQueue = renderQueue;
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

	const bool MaterialPass::IsInstanced() const
	{
		return m_instanced;
	}

	void MaterialPass::SubmitDrawCommands(GraphicsDevice& gfx, const Drawable& drawable, const DrawContext& drawContext, const BindingList* const pPropertyBindings) const
	{
		drawContext.renderer.AcceptDrawCall(gfx, std::move(DrawCommand(this, &drawable, drawContext, pPropertyBindings)), m_renderPass);
	}

	void MaterialPass::Bind(const GraphicsDevice & gfx, RenderState& renderState) const
	{
		m_pInputLayout->BindIA(gfx, renderState, 0u);
		m_pVertexShader->BindVS(gfx, renderState, 0u);
		if (m_pHullShader != nullptr)
		{
			m_pHullShader->BindHS(gfx, renderState, 0u);
		}
		if (m_pDomainShader != nullptr)
		{
			m_pDomainShader->BindDS(gfx, renderState, 0u);
		}
		if (m_pPixelShader != nullptr)
		{
			m_pPixelShader->BindPS(gfx, renderState, 0u);
		}
		if (m_pDepthStencilState != nullptr)
		{
			m_pDepthStencilState->BindOM(gfx, renderState);
		}
		for (const auto& b : m_bindings)
		{
			b.Bind(gfx, renderState);
		}
	}

}