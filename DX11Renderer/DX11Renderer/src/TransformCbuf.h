#pragma once
#include "Bindable.h"
#include "CommonHeader.h"
#include "ConstantBuffer.h"
#include "DXMathInclude.h"

namespace gfx
{
	class GraphicsDevice;
	class MeshRenderer;

	template<typename Type>
	class ConstantBuffer;

	struct Transforms;

	class TransformCbuf : public Bindable
	{
	public:
		TransformCbuf(GraphicsDevice& gfx);
		void BindVS(GraphicsDevice& gfx, UINT slot) override;
		void UpdateTransforms(GraphicsDevice& gfx, const Transforms& transforms);
	private:
		// Static so can be re-used each drawcall
		static std::unique_ptr<ConstantBuffer<Transforms>> pVcbuf;
	};
}