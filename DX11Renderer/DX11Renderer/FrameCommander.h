#pragma once
#include <array>
#include <string>
#include "BindableInclude.h"
#include "Stencil.h"
#include "Graphics.h"
#include "RenderJob.h"
#include "RenderPass.h"
#include "NullPixelShader.h"
#include "RenderTexture.h"

class FrameCommander
{
public:
	FrameCommander(Graphics& gfx)
	{
		pCameraColor = std::make_unique<RenderTexture>(gfx);
		renderPasses.emplace(GBufferRenderPassName, RenderPass());
	}
	void Accept(RenderJob job, std::string targetPass)
	{
		renderPasses[targetPass].EnqueueJob(job);
	}
	void Execute(Graphics& gfx)
	{
		// todo: replace w/ rendergraph

		// Can do global binds here

		// GBuffer pass
		{
			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);
			renderPasses[GBufferRenderPassName].Execute(gfx);
		}
		
		// outline masking pass
		/*Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Write)->Bind(gfx);
		Bind::NullPixelShader::Resolve(gfx)->Bind(gfx);
		passes[1].Execute(gfx);
		// outline drawing pass
		Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Mask)->Bind(gfx);
		passes[2].Execute(gfx);*/
	}
	void Reset()
	{
		for (auto& p : renderPasses)
		{
			p.second.Reset();
		}
	}
private:
	std::unique_ptr<RenderTexture> pCameraColor;
	std::unordered_map<std::string, RenderPass> renderPasses;
private:
	const std::string GBufferRenderPassName = std::string("GBuffer");
};