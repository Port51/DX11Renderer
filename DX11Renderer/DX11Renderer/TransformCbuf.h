#pragma once
#include "Bindable.h"
#include "WindowsInclude.h"
#include "ConstantBuffer.h"
#include "DXMathInclude.h"

class Graphics;
class MeshRenderer;

class TransformCbuf : public Bindable
{
protected:
	struct Transforms
	{
		dx::XMMATRIX model;
		dx::XMMATRIX modelView;
		dx::XMMATRIX modelViewProj;
	};
public:
	TransformCbuf(Graphics& gfx, const MeshRenderer& parent);
	void BindVS(Graphics& gfx, const RenderPass& renderPass, UINT slot) override;
	void InitializeParentReference(const MeshRenderer& parent) override;
protected:
	virtual void UpdateBindImpl(Graphics& gfx, const Transforms& transforms);
	Transforms GetTransforms(Graphics& gfx);
private:
	// Static so can be re-used each drawcall
	static std::unique_ptr<ConstantBuffer<Transforms>> pVcbuf;
	const MeshRenderer* pParent = nullptr;
};
