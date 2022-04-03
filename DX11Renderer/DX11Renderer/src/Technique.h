#pragma once
#include "CommonHeader.h"
#include "RenderStep.h"
#include <vector>
#include <string>

namespace gfx
{
	class MeshRenderer;
	class Renderer;
	struct DrawContext;

	class Technique
	{
	public:
		Technique() = default;
		Technique(std::string name);
		virtual ~Technique() = default;
	public:
		void SubmitDrawCalls(const MeshRenderer& meshRenderer, const DrawContext& drawContext) const;
		void AddStep(std::unique_ptr<RenderStep> step);
		bool IsActive() const;
		void SetActiveState(bool active_in);
		const std::string& GetName() const;
	private:
		bool m_active = true;
		std::vector<std::unique_ptr<RenderStep>> m_pSteps;
		std::string m_name = "Nameless Tech";
	};
}