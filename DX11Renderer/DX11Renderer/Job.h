#pragma once

class Job
{
public:
	Job(const class Step* pStep, const class MeshRenderer* pRenderer);
	void Execute(class Graphics& gfx) const;
private:
	const class MeshRenderer* pRenderer;
	const class Step* pStep;
};