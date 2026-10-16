#pragma once

#include <imgui.h>

namespace parallax {

enum class ButtonVariant {
    Primary = 0,               // Solid accent surface with contrast-safe text
    Secondary,                 // Tactile control surface with crisp border
    Danger,                    // Destructive action surface and border
    Ghost,                     // Subtle/transparent until hovered
    Subtle = Ghost,            // Alias for Ghost
    Destructive = Danger       // Alias for Danger
};

struct ButtonOptions {
    const char* icon{nullptr};
    ButtonVariant variant{ButtonVariant::Secondary};
    ImVec2 size{0.0f, 0.0f};
    bool compact{false};
    bool disabled{false};
    const char* tooltip{nullptr};
};

// Standard Parallax Button
bool Button(
    const char* label,
    const char* icon = nullptr,
    ButtonVariant variant = ButtonVariant::Secondary,
    const ImVec2& size = ImVec2(0.0f, 0.0f)
);

// Compact Parallax Button
bool ButtonCompact(
    const char* label,
    const char* icon = nullptr,
    ButtonVariant variant = ButtonVariant::Secondary,
    const ImVec2& size = ImVec2(0.0f, 0.0f)
);

// Icon-only Parallax Button
bool IconButton(
    const char* idLabel,
    const char* icon,
    ButtonVariant variant = ButtonVariant::Ghost,
    float dimension = 0.0f,
    const char* tooltip = nullptr
);

// Extended Parallax Button with full options
bool ButtonEx(const char* label, const ButtonOptions& options = ButtonOptions{});

} // namespace parallax
