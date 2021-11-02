#pragma once
#include <DirectXMath.h>
#include <type_traits>
#include "Graphics.h"
#include "Colors.h"

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
		Texture2D,
		Normal,
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
	};
	template<> struct Map<Position3D>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Position";
	};
	template<> struct Map<Texture2D>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Texcoord";
	};
	template<> struct Map<Normal>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Normal";
	};
	template<> struct Map<Float3Color>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Color";
	};
	template<> struct Map<Float4Color>
	{
		using SysType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char* semantic = "Color";
	};
	template<> struct Map<BGRAColor>
	{
		using SysType = Colors::BGRAColor;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr const char* semantic = "Color";
	};

	class Element
	{
	public:
		Element(ElementType type, size_t offset)
			:
			type(type),
			offset(offset)
		{}
		size_t GetOffsetAfter() const noexcept(!IS_DEBUG)
		{
			return offset + Size();
		}
		size_t GetOffset() const
		{
			return offset;
		}
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return SizeOf(type);
		}
		static constexpr size_t SizeOf(ElementType type) noexcept(!IS_DEBUG)
		{
			switch (type)
			{
			case Position2D:
				return sizeof(Map<Position2D>::SysType);
			case Position3D:
				return sizeof(Map<Position3D>::SysType);
			case Texture2D:
				return sizeof(Map<Texture2D>::SysType);
			case Normal:
				return sizeof(Map<Normal>::SysType);
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
		ElementType GetType() const noexcept
		{
			return type;
		}
		///
		/// Get layout description
		///
		D3D11_INPUT_ELEMENT_DESC GetDesc() const noexcept(!IS_DEBUG)
		{
			switch (type)
			{
			case Position2D:
				return GenerateDesc<Position2D>(GetOffset());
			case Position3D:
				return GenerateDesc<Position3D>(GetOffset());
			case Texture2D:
				return GenerateDesc<Texture2D>(GetOffset());
			case Normal:
				return GenerateDesc<Normal>(GetOffset());
			case Float3Color:
				return GenerateDesc<Float3Color>(GetOffset());
			case Float4Color:
				return GenerateDesc<Float4Color>(GetOffset());
			case BGRAColor:
				return GenerateDesc<BGRAColor>(GetOffset());
			}
			THROW_GFX_EXCEPT("Invalid element type");
			return { "INVALID",0,DXGI_FORMAT_UNKNOWN,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 };
		}
	private:
		template<ElementType type>
		static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(size_t offset) noexcept(!IS_DEBUG)
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
			return { Map<type>::semantic, 0, Map<type>::dxgiFormat, 0, (UINT)offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		}
	private:
		ElementType type;
		size_t offset;
	};
public:
	// Accessor
	template<ElementType Type>
	const Element& Resolve() const noexcept(!IS_DEBUG)
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
	const Element& ResolveByIndex(size_t i) const noexcept(!IS_DEBUG)
	{
		return elements[i];
	}
	VertexLayout& Append(ElementType type) noexcept(!IS_DEBUG)
	{
		elements.emplace_back(type, Size());
		return *this;
	}
	// Size in bytes
	size_t Size() const noexcept(!IS_DEBUG)
	{
		return elements.empty() ? 0u : elements.back().GetOffsetAfter();
	}
	size_t GetElementCount() const noexcept
	{
		return elements.size();
	}
	///
	/// Returns layout description, including shader semantics
	///
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const noexcept(!IS_DEBUG)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
		desc.reserve(GetElementCount());
		for (const auto& e : elements)
		{
			desc.push_back(e.GetDesc());
		}
		return desc;
	}
private:
	std::vector<Element> elements;
};