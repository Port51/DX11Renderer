#pragma once

class MeshRenderer;
class Graphics;

namespace Rendergraph
{
	class RenderStep;

	class RenderJob
	{
	public:
		RenderJob(const RenderStep* pStep, const MeshRenderer* pRenderer);
		void Execute(Graphics& gfx) const;
	private:
		const MeshRenderer* pRenderer;
		const RenderStep* pStep;
	};
}