#pragma once
#include "CommonHeader.h"
#include <string>

namespace gfx
{
	class GraphicsDevice;
	class MeshRenderer;
	class Renderer;
	class PixelShader;
	class VertexShader;
	class VertexLayout;
	class Binding;
	class InputLayout;
	class Bindable;
	class BindingList;

	struct DrawContext;

	enum RenderPassType : int;

	// Collection of bindables needed to execute a material pass in a technique
	class MaterialPass
	{
	public:
		MaterialPass();
		virtual void Release();
	public:
		void SetPixelShader(std::shared_ptr<PixelShader> pPixelShader);
		void SetVertexShader(std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<InputLayout> pInputLayout);
		//void SetVertexLayout(std::shared_ptr<VertexLayout> pVertexLayout);
		Binding& AddBinding(std::shared_ptr<Bindable> pBindable);
		const u64 GetMaterialCode() const;
		int GetPropertySlot() const;
		void SetPropertySlot(const int slotIdx);
		void SetRenderPass(const RenderPassType renderPass);
		const RenderPassType GetRenderPass() const;
		void SubmitDrawCommands(const MeshRenderer& meshRenderer, const DrawContext& drawContext, const BindingList* const pPropertyBindings) const;
		void Bind(const GraphicsDevice& gfx, RenderState& renderState) const;
	private:
		int m_propertySlotIdx = -1;
		RenderPassType m_renderPass;
		std::shared_ptr<InputLayout> m_pInputLayout;
		std::shared_ptr<VertexShader> m_pVertexShader;
		std::shared_ptr<PixelShader> m_pPixelShader;
		std::vector<Binding> m_bindings;
	};
}