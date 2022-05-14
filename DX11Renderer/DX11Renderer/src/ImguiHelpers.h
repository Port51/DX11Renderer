#pragma once

namespace gfx
{

	///
	/// Draws a button that's part of an array where one button can be selected
	/// An int controls which button is selected
	///
	int DrawSelectableButtonInArray(int mySelectionIdx, const char* label, int activeSelectionIdx, ImVec2& size, bool& changed, bool isSameLine = false);

	///
	/// Draws on and off buttons
	///
	bool DrawToggleOnOffButton(int id, const char* label, bool isSelected, ImVec2& size, bool& changed, bool isSameLine = false);

	///
	/// Draws slider float, with label to the left
	///
	bool DrawSliderFloat(int id, int indent, const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);

}