#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"
#include <string>

namespace gfx
{

	class VariableParser
	{
	public:
		// todo: make cpp file
		static const size_t GetByteSize(std::string typeName)
		{
			if (typeName == "float")
				return sizeof(float);
			else if (typeName == "float2")
				return sizeof(dx::XMFLOAT2);
			else if (typeName == "float3")
				return sizeof(dx::XMFLOAT3);
			else if (typeName == "float4")
				return sizeof(dx::XMFLOAT4);
			return 0u;
		}
	};

}