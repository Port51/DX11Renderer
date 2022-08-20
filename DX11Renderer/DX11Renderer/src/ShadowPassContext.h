#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"

namespace gfx
{
	class GraphicsDevice;
	class Camera;
	class Renderer;
	class RenderPass;
	class RendererList;
	class ConstantBuffer;

	struct GlobalTransformCB;

	class ShadowPassContext
	{
	public:
		ShadowPassContext(GraphicsDevice& gfx, const Camera& cam, Renderer& renderer, RenderPass& pRenderPass, ConstantBuffer& transformationCB, RendererList* pRendererList);
		virtual ~ShadowPassContext();
		//void Update();
	public:
		GraphicsDevice& gfx;
		const Camera& camera;
		Renderer& renderer;
		RenderPass& renderPass;
		ConstantBuffer& transformationCB;
		RendererList* pRendererList; // needs to be nullable
	};
}