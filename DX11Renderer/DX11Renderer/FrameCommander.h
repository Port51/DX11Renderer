#pragma once
#include <array>
#include <string>
#include "BindableInclude.h"
#include "Stencil.h"
#include "Graphics.h"
#include "Job.h"
#include "Pass.h"
#include "NullPixelShader.h"

class FrameCommander
{
public:
	void Accept(Job job, std::string targetPass) noexcept
	{
		passes[targetPass].Accept(job);
	}
	void Execute(Graphics& gfx)
	{
		// normally this would be a loop with each pass defining it setup / etc.
		// and later on it would be a complex graph with parallel execution contingent
		// on input / output requirements

		// Can do global binds here

		// GBuffer pass
		{
			Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Off)->Bind(gfx, 0u);
			passes[std::string("GBuffer")].Execute(gfx);
		}
		
		// outline masking pass
		/*Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Write)->Bind(gfx);
		Bind::NullPixelShader::Resolve(gfx)->Bind(gfx);
		passes[1].Execute(gfx);
		// outline drawing pass
		Bind::Stencil::Resolve(gfx, Bind::Stencil::Mode::Mask)->Bind(gfx);
		passes[2].Execute(gfx);*/
	}
	void Reset() noexcept
	{
		for (auto& p : passes)
		{
			p.second.Reset();
		}
	}
private:
	std::unordered_map<std::string, Pass> passes;
};