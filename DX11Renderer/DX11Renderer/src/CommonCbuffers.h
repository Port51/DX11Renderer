#pragma once
#include <DirectXMath.h>

namespace gfx
{
	struct alignas(16) LightInputCB
	{
		UINT visibleLightCount;
		dx::XMVECTOR shadowAtlasTexelResolution;
		dx::XMVECTOR shadowCascadeSphere0;
		dx::XMVECTOR shadowCascadeSphere1;
		dx::XMVECTOR shadowCascadeSphere2;
		dx::XMVECTOR shadowCascadeSphere3;
	};

	struct alignas(16) PerFrameCB
	{
		dx::XMVECTOR time; // (t/20, t, t*2, t*3)
	};

	struct alignas(16) GlobalTransformCB
	{
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projMatrix;
		dx::XMMATRIX viewProjMatrix;
		dx::XMMATRIX invViewMatrix;
		dx::XMMATRIX invProjMatrix;
		dx::XMMATRIX invViewProjMatrix;
	};

	struct alignas(16) PerCameraCB
	{
		// x = 1 or -1 (-1 if projection is flipped)
		// y = near plane
		// z = far plane
		// w = 1/far plane
		dx::XMVECTOR projectionParams;

		// x = width
		// y = height
		// z = 1 + 1.0/width
		// w = 1 + 1.0/height
		dx::XMVECTOR screenParams;

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
		dx::XMVECTOR zBufferParams;

		// x = orthographic camera's width
		// y = orthographic camera's height
		// z = unused
		// w = 1.0 if camera is ortho, 0.0 if perspective
		dx::XMVECTOR orthoParams;

		dx::XMVECTOR frustumCornerDataVS;
		dx::XMVECTOR cameraPositionWS;
		dx::XMVECTOR clusterPrecalc;
	};

	struct alignas(16) HiZCreationCB
	{
		dx::XMVECTORU32 resolutionSrcDst;
		//dx::XMVECTOR zBufferParams;
	};

	struct alignas(16) ClusteredLightingCB
	{
		dx::XMVECTORU32 groupResolutions; // groupsX, groupsY, groupsZ, padding
	};

	struct alignas(16) BlurPyramidCreationCB
	{
		dx::XMVECTORU32 resolutionSrcDst;
		//dx::XMVECTOR zBufferParams;
	};

	struct alignas(16) FXAA_CB
	{
		float minThreshold;
		float maxThreshold;
		float edgeSharpness;
		float padding;
	};

	struct alignas(16) SSR_CB
	{
		UINT debugViewStep;
		UINT padding0;
		UINT padding1;
		UINT padding2;
	};

	struct alignas(16) DitherCB
	{
		float shadowDither;
		float midDither;
		float padding0;
		float padding1;
	};
}