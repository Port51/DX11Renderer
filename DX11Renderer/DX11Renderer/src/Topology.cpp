#include "pch.h"
#include "Topology.h"
#include "SharedCodex.h"
#include "Graphics.h"

namespace gfx
{
	Topology::Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
		: type(type)
	{}

	void Topology::BindIA(Graphics& gfx, UINT slot)
	{
		gfx.GetContext()->IASetPrimitiveTopology(type);
	}

	std::shared_ptr<Topology> Topology::Resolve(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
	{
		return std::move(Codex::Resolve<Topology>(gfx, GenerateUID(type), type));
	}

	std::string Topology::GenerateUID(D3D11_PRIMITIVE_TOPOLOGY type)
	{
		using namespace std::string_literals;
		return typeid(Topology).name() + "#"s + std::to_string(type);
	}
}