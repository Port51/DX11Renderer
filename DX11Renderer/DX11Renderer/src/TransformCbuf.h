#pragma once
#include "Bindable.h"
#include <wrl.h>
#include <memory>
#include "ConstantBuffer.h"
#include "DXMathInclude.h"

class Graphics;
class MeshRenderer;

template<typename Type>
class ConstantBuffer;

struct Transforms;

class TransformCbuf : public Bindable
{
public:
	TransformCbuf(Graphics& gfx);
	void BindVS(Graphics& gfx, UINT slot) override;
	void UpdateTransforms(Graphics& gfx, const Transforms& transforms);
private:
	// Static so can be re-used each drawcall
	static std::unique_ptr<ConstantBuffer<Transforms>> pVcbuf;
};