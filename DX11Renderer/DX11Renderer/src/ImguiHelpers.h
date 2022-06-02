#pragma once

namespace gfx
{

	///
	/// Draws a button that's part of an array where one button can be selected
	/// An int controls which button is selected
	///
	int DrawSelectableButtonInArray(const int mySelectionIdx, const char* label, const int activeSelectionIdx, const ImVec2& size, bool& changed, const bool isSameLine = false);

	///
	/// Draws on and off buttons
	///
	bool DrawToggleOnOffButton(const int id, const char* label, const bool isSelected, const ImVec2& size, bool& changed, const bool isSameLine = false);

	///
	/// Draws slider float, with label to the left
	///
	bool DrawSliderFloat(const int id, const int indent, const char* label, float* v, const float v_min, const float v_max, const char* format, const ImGuiSliderFlags flags);

}