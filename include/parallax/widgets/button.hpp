#pragma once

#include <imgui.h>

namespace parallax {

enum class ButtonVariant {
    Primary,    // Accent gradient / solid with bright text
    Secondary,  // Dark slate card surface with subtle border
    Danger,     // Crimson tinted surface and border
    Ghost       // Transparent background until hovered
};

// Custom Parallax Button with spring-animated hover/active states and SVG icon support
bool Button(const char* label, const char* icon = nullptr, ButtonVariant variant = ButtonVariant::Secondary, const ImVec2& size = ImVec2(0.0f, 0.0f));

} // namespace parallax
