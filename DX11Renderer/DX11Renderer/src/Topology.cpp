#include "pch.h"
#include "Topology.h"
#include "SharedCodex.h"
#include "GraphicsDevice.h"

namespace gfx
{
	Topology::Topology(const GraphicsDevice& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
		: m_type(type)
	{}

	void Topology::Release()
	{
	}

	void Topology::BindIA(const GraphicsDevice& gfx, RenderState& renderState, UINT slot)
	{
		if (renderState.IsNewBinding(m_type, RenderBindingType::IA_Topology, 0u))
		{
			gfx.GetContext()->IASetPrimitiveTopology(m_type);
		}
	}

	void Topology::UnbindIA(const GraphicsDevice & gfx, RenderState & renderState, UINT slot)
	{
		renderState.ClearBinding(RenderBindingType::IA_Topology, 0u);
		gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED);
	}

	std::shared_ptr<Topology> Topology::Resolve(const GraphicsDevice& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
	{
		return std::move(Codex::Resolve<Topology>(gfx, GenerateUID(type), type));
	}

	std::string Topology::GenerateUID(D3D11_PRIMITIVE_TOPOLOGY type)
	{
		using namespace std::string_literals;
		return typeid(Topology).name() + "#"s + std::to_string(type);
	}
}