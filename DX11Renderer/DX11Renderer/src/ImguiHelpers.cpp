#include "pch.h"
#include "ImguiHelpers.h"

///
/// Draws a button that's part of an array where one button can be selected
/// An int controls which button is selected
///
int gfx::DrawSelectableButtonInArray(int mySelectionIdx, const char * label, int activeSelectionIdx, ImVec2& size, bool& changed, bool isSameLine)
{
	static ImVec4 selectedColor = { 1, 1, 1, 0.5 };

	ImGui::PushID(mySelectionIdx + 13273);
	bool isSelected = (mySelectionIdx == activeSelectionIdx);
	int newSelectionIdx = activeSelectionIdx;

	if (isSelected)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
	}
	if (isSameLine)
	{
		ImGui::SameLine();
	}
	if (ImGui::Button(label, size))
	{
		newSelectionIdx = mySelectionIdx;
		changed = true;
	}
	if (isSelected)
	{
		ImGui::PopStyleColor(1);
	}
	ImGui::PopID();

	return newSelectionIdx;
}

bool gfx::DrawToggleOnOffButton(int id, const char* label, bool isSelected, ImVec2& size, bool& changed, bool isSameLine)
{
	static ImVec4 selectedColor = { 1, 1, 1, 0.5 };

	bool returnValue = isSelected;
	if (isSameLine)
	{
		ImGui::SameLine();
	}
	ImGui::PushID(id + 3048);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text(label);
	ImGui::SetCursorPosX(500);
	
	// Off button
	{
		if (!isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
		}
		ImGui::TableSetColumnIndex(1);
		if (ImGui::Button("OFF", size))
		{
			returnValue = false;
			changed = true;
		}
		if (!isSelected)
		{
			ImGui::PopStyleColor(1);
		}
	}

	// On button
	{
		if (isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
		}
		ImGui::SameLine();
		if (ImGui::Button("ON", size))
		{
			returnValue = true;
			changed = true;
		}
		if (isSelected)
		{
			ImGui::PopStyleColor(1);
		}
	}
	ImGui::PopID();
	
	return returnValue;
}

bool gfx::DrawSliderFloat(int id, int indent, const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
	ImGui::PushID(id + 3048);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::SetCursorPosX(indent);
	ImGui::Text(label);
	ImGui::SetCursorPosX(indent + 500);

	ImGui::TableSetColumnIndex(1);
	bool returnValue = ImGui::SliderFloat("", v, v_min, v_max, format, flags);

	ImGui::PopID();
	return returnValue;
}

