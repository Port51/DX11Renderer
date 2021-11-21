#pragma once
#include <DirectXMath.h>
#include <type_traits>
#include "Graphics.h"
#include "Colors.h"
#include "GraphicsThrowMacros.h"

///
/// Descriptor class for layout required by a vertex shader
///
class VertexLayout
{
public:
	enum ElementType
	{
		Position2D,
		Position3D,
		Texcoord2D,
		Normal,
		Tangent,
		Float3Color,
		Float4Color,
		BGRAColor,
		Count,
	};
	// template metaprogramming
	// make struct templated on ElementType
	// becomes compile-time lookup
	// and allows storing all settings in one place
	template<ElementType> struct Map;

	// "Template specializations" in above Map
	template<> struct Map<Position2D>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Position";
		static constexpr const char* code = "P2";
	};
	template<> struct Map<Position3D>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Position";
		static constexpr const char* code = "P3";
	};
	template<> struct Map<Texcoord2D>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Texcoord";
		static constexpr const char* code = "TC2";
	};
	template<> struct Map<Normal>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Normal";
		static constexpr const char* code = "N";
	};
	template<> struct Map<Tangent>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Tangent";
		static constexpr const char* code = "TG";
	};
	template<> struct Map<Float3Color>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C3";
	};
	template<> struct Map<Float4Color>
	{
		using SysType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C4";
	};
	template<> struct Map<BGRAColor>
	{
		using SysType = Colors::BGRAColor;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C8";
	};

	class Element
	{
	public:
		Element(ElementType type, size_t offset, UINT semanticIndex, D3D11_INPUT_CLASSIFICATION classification)
			:
			type(type),
			offset(offset),
			semanticIndex(semanticIndex),
			classification(classification)
		{}
		size_t GetOffsetAfter() const
		{
			return offset + Size();
		}
		size_t GetOffset() const
		{
			return offset;
		}
		UINT GetSemanticIndex() const
		{
			return semanticIndex;
		}
		size_t Size() const
		{
			return SizeOf(type);
		}
		static constexpr size_t SizeOf(ElementType type)
		{
			switch (type)
			{
			case Position2D:
				return sizeof(Map<Position2D>::SysType);
			case Position3D:
				return sizeof(Map<Position3D>::SysType);
			case Texcoord2D:
				return sizeof(Map<Texcoord2D>::SysType);
			case Normal:
				return sizeof(Map<Normal>::SysType);
			case Tangent:
				return sizeof(Map<Tangent>::SysType);
			case Float3Color:
				return sizeof(Map<Float3Color>::SysType);
			case Float4Color:
				return sizeof(Map<Float4Color>::SysType);
			case BGRAColor:
				return sizeof(Map<BGRAColor>::SysType);
			}
			THROW_GFX_EXCEPT("Invalid element type");
			return 0u;
		}
		ElementType GetType() const
		{
			return type;
		}
		///
		/// Get layout description
		///
		D3D11_INPUT_ELEMENT_DESC GetDesc() const
		{
			switch (type)
			{
			case Position2D:
				return GenerateDesc<Position2D>(offset, semanticIndex, classification);
			case Position3D:
				return GenerateDesc<Position3D>(offset, semanticIndex, classification);
			case Texcoord2D:
				return GenerateDesc<Texcoord2D>(offset, semanticIndex, classification);
			case Normal:
				return GenerateDesc<Normal>(offset, semanticIndex, classification);
			case Tangent:
				return GenerateDesc<Tangent>(offset, semanticIndex, classification);
			case Float3Color:
				return GenerateDesc<Float3Color>(offset, semanticIndex, classification);
			case Float4Color:
				return GenerateDesc<Float4Color>(offset, semanticIndex, classification);
			case BGRAColor:
				return GenerateDesc<BGRAColor>(offset, semanticIndex, classification);
			}
			THROW_GFX_EXCEPT("Invalid element type");
			return { "INVALID",0,DXGI_FORMAT_UNKNOWN,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 };
		}
		const char* GetCode() const
		{
			switch (type)
			{
			case Position2D:
				return Map<Position2D>::code;
			case Position3D:
				return Map<Position3D>::code;
			case Texcoord2D:
				return Map<Texcoord2D>::code;
			case Normal:
				return Map<Normal>::code;
			case Tangent:
				return Map<Tangent>::code;
			case Float3Color:
				return Map<Float3Color>::code;
			case Float4Color:
				return Map<Float4Color>::code;
			case BGRAColor:
				return Map<BGRAColor>::code;
			}
			THROW_GFX_EXCEPT("Invalid element type");
			return "INVALID";
		}

	private:
		template<ElementType type>
		static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(size_t offset, UINT semanticIndex, D3D11_INPUT_CLASSIFICATION classification)
		{
			// IN ORDER:
			// Semantic "Position" must match vertex shader semantic
			// The 0 after = index of semantic
			// Format = simple...
			// Slot = ...
			// Offset in bytes in structure
			// Vert vs. instances
			// Instance stuff
			// EXAMPLE: { "Position", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			return { Map<type>::semantic, semanticIndex, Map<type>::dxgiFormat, 0, (UINT)offset, classification, 0 };
		}
	private:
		ElementType type;
		size_t offset;
		UINT semanticIndex;
		D3D11_INPUT_CLASSIFICATION classification;
	};

public:
	// Accessor
	template<ElementType Type>
	const Element& Resolve() const
	{
		for (auto& e : elements)
		{
			if (e.GetType() == Type)
			{
				return e;
			}
		}
		THROW_GFX_EXCEPT("Could not resolve element type");
		return elements.front();
	}
	const Element& ResolveByIndex(size_t i) const
	{
		return elements[i];
	}
	VertexLayout& Append(ElementType type, UINT semanticIndex = 0u, D3D11_INPUT_CLASSIFICATION classification = D3D11_INPUT_PER_VERTEX_DATA)
	{
		elements.emplace_back(type, Size(), semanticIndex, classification);
		return *this;
	}
	// Size in bytes
	size_t Size() const
	{
		return elements.empty() ? 0u : elements.back().GetOffsetAfter();
	}
	size_t GetElementCount() const
	{
		return elements.size();
	}
	///
	/// Returns layout description, including shader semantics
	///
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
		desc.reserve(GetElementCount());
		for (const auto& e : elements)
		{
			desc.push_back(e.GetDesc());
		}
		return desc;
	}

	std::string GetCode() const
	{
		std::string code;
		for (const auto& e : elements)
		{
			code += e.GetCode();
		}
		return code;
	}

private:
	std::vector<Element> elements;
};