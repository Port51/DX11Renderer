#pragma once

class RenderJob
{
public:
	RenderJob(const class RenderStep* pStep, const class MeshRenderer* pRenderer);
	void Execute(class Graphics& gfx) const;
private:
	const class MeshRenderer* pRenderer;
	const class RenderStep* pStep;
};