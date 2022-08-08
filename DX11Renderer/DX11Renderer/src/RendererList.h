#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Renderer;
	class MeshRenderer;
	class BaseModel;
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
		virtual ~RendererList();
	public:
		const UINT GetRendererCount() const;
		void Filter(const GraphicsDevice& gfx, const Frustum& frustum, const RendererSortingType sorting, const RenderPassType renderPassType, const dx::XMVECTOR originWS, const dx::XMVECTOR directionWS, const float farClipPlane);
		void SubmitDrawCalls(GraphicsDevice& gfx, const DrawContext& drawContext) const;
		void AddModel(const BaseModel& model);
		void AddSceneGraph(std::shared_ptr<SceneGraphNode> pSceneGraph);
	private:
		static bool SortByCode(const std::pair<std::shared_ptr<MeshRenderer>, u64>& a, const std::pair<std::shared_ptr<MeshRenderer>, u64>& b);
	private:
		// Float measures view distance
		std::shared_ptr<RendererList> m_pSource;
		std::vector<std::shared_ptr<SceneGraphNode>> m_pSceneGraphs;
		std::vector<std::pair<std::shared_ptr<MeshRenderer>, u64>> m_pRenderers;
	};
}