#pragma once
#include "CommonHeader.h"
#include <string>

namespace gfx
{
	class GraphicsDevice;
	class Drawable;
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
		const int GetPropertySlot() const;
		void SetPropertySlot(const int slotIdx);
		const int GetRenderQueue() const;
		void SetRenderQueue(const u8 renderQueue);
		void SetRenderPass(const RenderPassType renderPass);
		const RenderPassType GetRenderPass() const;
		void SubmitDrawCommands(const Drawable& drawable, const DrawContext& drawContext, const BindingList* const pPropertyBindings) const;
		void Bind(const GraphicsDevice& gfx, RenderState& renderState) const;
	private:
		int m_propertySlotIdx = -1;
		u8 m_renderQueue = 0;
		RenderPassType m_renderPass;
		std::shared_ptr<InputLayout> m_pInputLayout;
		std::shared_ptr<VertexShader> m_pVertexShader;
		std::shared_ptr<PixelShader> m_pPixelShader;
		std::vector<Binding> m_bindings;
	};
}