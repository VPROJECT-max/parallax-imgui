#pragma once

#include <imgui.h>

namespace parallax {

bool BeginCollapsibleGroupbox(
    const char* title,
    bool* p_expanded,
    const char* icon = nullptr,
    float defaultHeight = 0.0f,
    bool centered = true
);

void EndCollapsibleGroupbox();

} // namespace parallax
