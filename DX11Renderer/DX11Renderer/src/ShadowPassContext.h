#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"

namespace gfx
{
	class Graphics;
	class Camera;
	class Renderer;
	class RenderPass;
	class RendererList;

	struct GlobalTransformCB;

	template<typename Type>
	class ConstantBuffer;

	class ShadowPassContext
	{
	public:
		ShadowPassContext(Graphics& gfx, const std::unique_ptr<Camera>& cam, Renderer& renderer, const std::unique_ptr<RenderPass>& pRenderPass, std::unique_ptr<ConstantBuffer<GlobalTransformCB>>& pTransformationCB, std::shared_ptr<RendererList> pRendererList);
		virtual ~ShadowPassContext() = default;
	public:
		Graphics& gfx;
		const std::unique_ptr<Camera>& pCamera;
		dx::XMMATRIX invViewMatrix;
		Renderer& renderer;
		const std::unique_ptr<RenderPass>& pRenderPass;
		std::unique_ptr<ConstantBuffer<GlobalTransformCB>>& pTransformationCB;
		std::shared_ptr<RendererList> pRendererList;
	};
}