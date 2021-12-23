#pragma once
#include "Bindable.h"
#include "WindowsInclude.h"
#include "ConstantBuffer.h"
#include "DXMathInclude.h"

class Graphics;
class MeshRenderer;
struct Transforms;

class TransformCbuf : public Bindable
{
public:
	TransformCbuf(Graphics& gfx, const MeshRenderer& parent);
	void BindVS(Graphics& gfx, UINT slot) override;
	void BindVS2(Graphics& gfx, UINT slot, Transforms transforms, dx::XMMATRIX model, const MeshRenderer& parent);
	void InitializeParentReference(const MeshRenderer& parent) override;
	void UpdateBindImpl(Graphics& gfx, const Transforms& transforms);
private:
	// Static so can be re-used each drawcall
	static std::unique_ptr<ConstantBuffer<Transforms>> pVcbuf;
	const MeshRenderer* pParent = nullptr;
};
