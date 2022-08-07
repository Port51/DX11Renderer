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
	class DepthStencilState;

	struct DrawContext;

	enum RenderPassType : int;

	// Collection of bindables needed to execute a material pass in a technique
	class MaterialPass
	{
	public:
		MaterialPass();
		virtual void Release();

	public:
		void SubmitDrawCommands(const Drawable& drawable, const DrawContext& drawContext, const BindingList* const pPropertyBindings) const;
		void Bind(const GraphicsDevice& gfx, RenderState& renderState) const;

	public:
		void SetInstanced(const bool isInstanced);
		void SetPixelShader(std::shared_ptr<PixelShader> pPixelShader);
		void SetVertexShader(std::shared_ptr<VertexShader> pVertexShader, std::shared_ptr<InputLayout> pInputLayout);
		void SetStencil(std::shared_ptr<DepthStencilState> pDepthStencilState);
		void SetPropertySlot(const int slotIdx);
		void SetRenderQueue(const u8 renderQueue);
		void SetRenderPass(const RenderPassType renderPass);
		void AddShaderDefine(std::string define);
		Binding& AddBinding(std::shared_ptr<Bindable> pBindable);

	public:
		const std::shared_ptr<DepthStencilState> GetStencil() const;
		const std::vector<std::string>& GetShaderDefines() const;
		const u64 GetMaterialCode() const;
		const int GetPropertySlot() const;
		const int GetRenderQueue() const;
		const RenderPassType GetRenderPass() const;
		const bool IsInstanced() const;
		
	private:
		int m_propertySlotIdx = -1;
		u8 m_renderQueue = 0;
		bool m_instanced = false;
		RenderPassType m_renderPass;
		std::shared_ptr<InputLayout> m_pInputLayout;
		std::shared_ptr<VertexShader> m_pVertexShader;
		std::shared_ptr<PixelShader> m_pPixelShader;
		std::shared_ptr<DepthStencilState> m_pDepthStencilState;
		std::vector<Binding> m_bindings;
		std::vector<std::string> m_shaderDefines;
	};
}