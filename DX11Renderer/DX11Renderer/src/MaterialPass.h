#pragma once
#include <string>

namespace gfx
{
	class Technique;
	class MeshRenderer;
	class Renderer;
	class PixelShader;
	class VertexShader;
	class VertexLayout;

	struct DrawContext;

	///
	/// Collection of bindables needed to execute a material pass in a technique
	///
	class MaterialPass
	{
	public:
		MaterialPass();
	public:
		void SetPixelShader(std::shared_ptr<PixelShader> pPixelShader);
		void SetVertexShader(std::shared_ptr<VertexShader> pVertexShader);
		void SetVertexLayout(std::shared_ptr<VertexLayout> pVertexLayout);
	public:
		void SetRenderPass(std::string renderPass);
		const std::string GetRenderPass() const;
		void AddTechnique(std::unique_ptr<Technique> _pTechnique);
		void SubmitDrawCommands(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
	private:
		std::unique_ptr<Technique> m_pTechnique;
		std::string m_renderPass;
	};
}