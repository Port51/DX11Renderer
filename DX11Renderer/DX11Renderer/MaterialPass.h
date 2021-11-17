#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include "VertexLayout.h"
#include "Bindable.h"
#include "Technique.h"

namespace Rendergraph
{
	class Technique;
	class FrameCommander;
}

namespace Bind
{
	class VertexShader;
	class PixelShader;
	class Material;
	class Binding;

	///
	/// Collection of bindables needed to execute a material pass in a technique
	///
	class MaterialPass
	{
	public:
		MaterialPass();
	public:
		void AddTechnique(std::unique_ptr<Rendergraph::Technique> _pTechnique);
		void SubmitDrawCalls(Rendergraph::FrameCommander& frame, const MeshRenderer& renderer) const;
	public:
		std::shared_ptr<VertexShader> pVertexShader;
		std::shared_ptr<PixelShader> pPixelShader;
	private:
		Material* pParent;
		std::unique_ptr<Rendergraph::Technique> pTechnique;
	};
}