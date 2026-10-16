#pragma once

#include <functional>
#include <imgui.h>

namespace parallax {

enum class ToggleStyle : uint8_t {
    Checkbox = 0,
    PillSwitch
};

struct ToggleOptions {
    const char* description{nullptr};
    const char* icon{nullptr};
    bool disabled{false};
    bool compact{false};
    bool indeterminate{false};
    ToggleStyle style{ToggleStyle::Checkbox};
    std::function<void(bool)> onChange{nullptr};
};

bool Checkbox(
    const char* label,
    bool* v,
    const char* description = nullptr,
    const char* icon = nullptr,
    bool disabled = false
);

bool CheckboxIndeterminate(
    const char* label,
    bool* v,
    bool indeterminate,
    const char* description = nullptr,
    const char* icon = nullptr,
    bool disabled = false
);

bool Toggle(
    const char* label,
    bool* v,
    const char* description = nullptr,
    const char* icon = nullptr,
    bool disabled = false
);

bool ToggleCompact(
    const char* label,
    bool* v,
    const char* description = nullptr,
    const char* icon = nullptr,
    bool disabled = false
);

bool ToggleEx(
    const char* label,
    bool* v,
    const ToggleOptions& options = ToggleOptions{}
);

// Shared custom control surface primitives used by ToggleEx and ControlRow
void DrawParallaxSwitchGlyph(
    ImDrawList* drawList,
    const ImVec2& switchMin,
    const ImVec2& switchMax,
    float checkProgress,
    float hoverProgress,
    bool held,
    bool disabled
);

void DrawParallaxCheckboxGlyph(
    ImDrawList* drawList,
    const ImVec2& boxMin,
    const ImVec2& boxMax,
    float checkProgress,
    float hoverProgress,
    bool indeterminate,
    bool disabled
);

} // namespace parallax
