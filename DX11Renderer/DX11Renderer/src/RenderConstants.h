#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class RenderSlots
	{
	public:
		static const UINT CS_PerFrameCB = 0u;
		static const UINT CS_TransformationCB = 1u;
		static const UINT CS_PerCameraCB = 2u;
		static const UINT CS_LightInputCB = 3u;
		static const UINT CS_FreeCB = 4u; // 1st available general-purpose slot

		static const UINT CS_LightDataSRV = 0u;
		static const UINT CS_LightShadowDataSRV = 1u;
		static const UINT CS_GbufferNormalRoughSRV = 2u;
		static const UINT CS_FreeSRV = 3u; // 1st available general-purpose slot
		static const UINT CS_FreeSPL = 0u; // 1st available general-purpose slot

		static const UINT CS_FreeUAV = 0u; // 1st available general-purpose slot

		static const UINT VS_PerFrameCB = 0u;
		static const UINT VS_TransformationCB = 1u;
		static const UINT VS_PerCameraCB = 2u;
		static const UINT VS_TransformCB = 3u;
		static const UINT VS_FreeCB = 4u; // 1st available general-purpose slot
		static const UINT VS_FreeSPL = 0u; // 1st available general-purpose slot

		static const UINT VS_FreeSRV = 0u; // 1st available general-purpose slot

		static const UINT PS_PerFrameCB = 0u;
		static const UINT PS_TransformationCB = 1u;
		static const UINT PS_PerCameraCB = 2u;
		static const UINT PS_LightInputCB = 3u;
		static const UINT PS_FreeCB = 4u; // 1st available general-purpose slot
		static const UINT PS_FreeSPL = 0u; // 1st available general-purpose slot

		// These are the first slots available for renderers
		static const UINT VS_FreeRendererCB = 0u;
		static const UINT VS_FreeRendererSRV = 0u;
		static const UINT VS_FreeRendererSPL = 0u;
		static const UINT PS_FreeRendererCB = 5u;
		static const UINT PS_FreeRendererSRV = 8u;
		static const UINT PS_FreeRendererSPL = 1u;

		static const UINT PS_FreeSRV = 2u; // 1st available general-purpose slot
	};

	class RenderConstants
	{
	public:
		// These are defined in the cpp file
		static const float PI;
		static const float PI_2;
		static const float INV_PI;
		static const float INV_PI_2;
		static const float DEG_TO_RAD;
		static const float RAD_TO_DEG;
	};
}