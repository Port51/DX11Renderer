#pragma once
#include <string>

namespace gfx
{
	class Technique;
	class MeshRenderer;
	class Renderer;

	struct DrawContext;

	///
	/// Collection of bindables needed to execute a material pass in a technique
	///
	class MaterialPass
	{
	public:
		MaterialPass();
	public:
		void SetRenderPass(std::string renderPass);
		const std::string GetRenderPass() const;
		void AddTechnique(std::unique_ptr<Technique> _pTechnique);
		void SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
	private:
		std::unique_ptr<Technique> pTechnique;
		std::string renderPass;
	};
}