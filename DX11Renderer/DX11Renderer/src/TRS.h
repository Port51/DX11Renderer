#pragma once
#include "CommonHeader.h"
#include "DXMathInclude.h"

namespace gfx
{
	struct TRS
	{
		dx::XMFLOAT3 position;
		dx::XMFLOAT3 rotation;
		dx::XMFLOAT3 scale;
		dx::XMFLOAT4X4 trs;

		TRS()
		{
			position = dx::XMFLOAT3(0.f, 0.f, 0.f);
			rotation = dx::XMFLOAT3(0.f, 0.f, 0.f);
			scale = dx::XMFLOAT3(0.f, 0.f, 0.f);
			dx::XMStoreFloat4x4(&trs, dx::XMMatrixIdentity());
		}
	};
}