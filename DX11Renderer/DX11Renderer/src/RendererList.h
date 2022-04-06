#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Renderer;
	class MeshRenderer;
	class ModelInstance;
	class SceneGraphNode;
	class GraphicsDevice;

	struct Frustum;
	struct DrawContext;

	class RendererList
	{
	public:
		enum RendererSorting
		{
			BackToFront,
			FrontToBack
		};
	public:
		RendererList() {}
		RendererList(std::shared_ptr<RendererList> source);
		virtual ~RendererList() = default;
	public:
		const UINT GetRendererCount() const;
		void Filter(GraphicsDevice& gfx, const Frustum& frustum, RendererSorting sorting);
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void AddModelInstance(const ModelInstance& modelInstance);
		void AddSceneGraph(const std::shared_ptr<SceneGraphNode> pSceneGraph);
	private:
		static bool SortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
		static bool SortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
	private:
		// Float measures view distance
		std::shared_ptr<RendererList> m_pSource;
		std::vector<std::shared_ptr<SceneGraphNode>> m_pSceneGraphs;
		std::vector<std::pair<std::shared_ptr<MeshRenderer>, float>> m_pRenderers;
	};
}