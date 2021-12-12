#pragma once
#include <wrl.h>

class RenderSlots
{
public:
	static const UINT CS_PerFrameCB = 0u;
	static const UINT CS_PerCameraCB = 1u;
	static const UINT CS_LightInputCB = 2u;
	static const UINT CS_FreeCB = 3u; // 1st available general-purpose slot

	static const UINT CS_LightDataSRV = 0u;
	static const UINT CS_GbufferNormalRoughSRV = 1u;
	static const UINT CS_FreeSRV = 2u; // 1st available general-purpose slot

	static const UINT CS_FreeUAV = 0u; // 1st available general-purpose slot

	static const UINT VS_PerFrameCB = 0u;
	static const UINT VS_PerCameraCB = 1u;
	static const UINT VS_TransformCB = 2u;
	static const UINT VS_FreeCB = 3u; // 1st available general-purpose slot

	static const UINT VS_FreeSRV = 0u; // 1st available general-purpose slot

	static const UINT PS_PerFrameCB = 0u;
	static const UINT PS_PerCameraCB = 1u;
	static const UINT PS_FreeCB = 2u; // 1st available general-purpose slot

	static const UINT PS_FreeSRV = 0u; // 1st available general-purpose slot
};