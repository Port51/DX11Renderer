#pragma once

#include <wrl.h>

struct BindSlots
{
	UINT CS_CB;
	UINT CS_UAV;
	UINT CS_SRV;
	UINT VS_CB;
	UINT VS_UAV;
	UINT VS_SRV;
	UINT PS_CB;
	UINT PS_UAV;
	UINT PS_SRV;
};