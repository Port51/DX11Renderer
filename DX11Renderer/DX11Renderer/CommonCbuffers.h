#pragma once
#include <DirectXMath.h>

struct alignas(16) LightInputCB
{
	UINT visibleLightCount;
};

struct alignas(16) PerFrameCB
{
	DirectX::XMVECTOR time; // (t/20, t, t*2, t*3)
};

struct alignas(16) PerCameraCB
{
	// x = 1 or -1 (-1 if projection is flipped)
	// y = near plane
	// z = far plane
	// w = 1/far plane
	DirectX::XMVECTOR projectionParams;

	// x = width
	// y = height
	// z = 1 + 1.0/width
	// w = 1 + 1.0/height
	DirectX::XMVECTOR screenParams;

	// Values used to linearize the Z buffer (http://www.humus.name/temp/Linearize%20depth.txt)
	// x = 1-far/near
	// y = far/near
	// z = x/far
	// w = y/far
	// or in case of a reversed depth buffer (UNITY_REVERSED_Z is 1)
	// x = -1+far/near
	// y = 1
	// z = x/far
	// w = 1/far
	DirectX::XMVECTOR zBufferParams;

	// x = orthographic camera's width
	// y = orthographic camera's height
	// z = unused
	// w = 1.0 if camera is ortho, 0.0 if perspective
	DirectX::XMVECTOR orthoParams;

	DirectX::XMVECTOR frustumCornerDataVS;

	DirectX::XMMATRIX viewProj;
	DirectX::XMMATRIX projection;
};