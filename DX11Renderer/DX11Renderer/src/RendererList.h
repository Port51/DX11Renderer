#pragma once
#include "CommonHeader.h"
#include <vector>

namespace gfx
{
	class Renderer;
	class MeshRenderer;
	class ModelInstance;

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
	public:
		UINT GetRendererCount() const;
		void Filter(Frustum frustum, RendererSorting sorting);
		void SubmitDrawCalls(const DrawContext& drawContext) const;
		void AddModelInstance(const ModelInstance& modelInstance);
		void AddMeshRenderer(const std::shared_ptr<MeshRenderer> pMeshRenderer);
	private:
		static bool SortFrontToBack(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
		static bool SortBackToFront(const std::pair<std::shared_ptr<MeshRenderer>, float>& a, const std::pair<std::shared_ptr<MeshRenderer>, float>& b);
	private:
		// Float measures view distance
		std::shared_ptr<RendererList> pSource;
		std::vector<std::pair<std::shared_ptr<MeshRenderer>, float>> pRenderers;
	};
}