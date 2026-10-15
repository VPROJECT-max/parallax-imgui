#pragma once

#include <imgui.h>

namespace parallax {

// Standalone compact color swatch button opening an animated color picker popup
bool ColorPicker(const char* str_id, ImVec4* color, bool hasAlpha = true, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool ColorPicker(const char* str_id, float* col3or4, bool hasAlpha = true, const ImVec2& size = ImVec2(0.0f, 0.0f));

} // namespace parallax
