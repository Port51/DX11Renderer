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
		enum RendererSortingType
		{
			StateThenBackToFront,
			StateThenFrontToBack,
			BackToFrontThenState,
			FrontToBackThenState,
			State
		};
	public:
		RendererList() {}
		RendererList(std::shared_ptr<RendererList> source);
		virtual ~RendererList() = default;
	public:
		const UINT GetRendererCount() const;
		void Filter(const GraphicsDevice& gfx, const Frustum& frustum, RendererSortingType sorting, dx::XMVECTOR originWS, dx::XMVECTOR directionWS, float farClipPlane);
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void AddModelInstance(const ModelInstance& modelInstance);
		void AddSceneGraph(const std::shared_ptr<SceneGraphNode> pSceneGraph);
	private:
		static bool SortByCode(const std::pair<std::shared_ptr<MeshRenderer>, u64>& a, const std::pair<std::shared_ptr<MeshRenderer>, u64>& b);
	private:
		// Float measures view distance
		std::shared_ptr<RendererList> m_pSource;
		std::vector<std::shared_ptr<SceneGraphNode>> m_pSceneGraphs;
		std::vector<std::pair<std::shared_ptr<MeshRenderer>, u64>> m_pRenderers;
	};
}