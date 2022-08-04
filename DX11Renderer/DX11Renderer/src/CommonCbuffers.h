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
		dx::XMVECTORU32 pixelSelection;
		dx::XMVECTOR time; // (t/20, t, t*2, t*3)
		dx::XMVECTOR sinTime; // sin(t/8), sin(t/4), sin(t/2), sin(t)
		dx::XMVECTOR cosTime; // cos(t/8), cos(t/4), cos(t/2), cos(t)
		dx::XMVECTOR timeStep; // so far, all components just contain the timestep
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

	struct alignas(16) ObjectTransformsCB
	{
		dx::XMMATRIX model;
		dx::XMMATRIX modelView;
		dx::XMMATRIX modelViewProj;
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
		dx::XMVECTOR inverseFrustumCornerDataVS;
		dx::XMVECTOR frustumPlaneDirVS;
		dx::XMVECTOR cameraPositionWS;
		dx::XMVECTOR clusterPrecalc;
		dx::XMVECTOR clusterXYRemap;
		u32 useOcclusion;
		float padding0;
		float padding1;
		float padding2;
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

	struct alignas(16) SSAO_CB
	{
		float radiusVS;
		float biasVS;
		float intensity;
		float sharpness;
	};

	struct alignas(16) DepthOfFieldCB
	{
		u32 weightOffset;
		float verticalPassAddFactor; // if 0, will overwrite. if 1, will add.
		float combineRealFactor;
		float combineImaginaryFactor;
		float nearCoCScale;
		float nearCoCBias;
		float nearCoCIntensity;
		float farCoCScale;
		float farCoCBias;
		float farCoCIntensity;
		float padding0;
		float padding1;
	};

	struct alignas(16) BloomCB
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
		u32 debugViewStep;
		u32 padding0;
		u32 padding1;
		u32 padding2;
	};

	struct alignas(16) DitherCB
	{
		float shadowDither;
		float midDither;
		float padding0;
		float padding1;
	};
}