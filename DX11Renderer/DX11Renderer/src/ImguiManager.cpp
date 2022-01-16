#include "pch.h"
#include "ImguiManager.h"

namespace gfx
{
	ImguiManager::ImguiManager()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
	}

	ImguiManager::~ImguiManager()
	{
		ImGui::DestroyContext();
	}
}