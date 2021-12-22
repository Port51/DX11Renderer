#pragma once

class RenderStep;
class RenderPass;
class MeshRenderer;
class Graphics;

class DrawCall
{
public:
	DrawCall(const RenderStep* pStep, const MeshRenderer* pRenderer);
	void Execute(Graphics& gfx) const;
private:
	const MeshRenderer* pRenderer;
	const RenderStep* pStep;
};