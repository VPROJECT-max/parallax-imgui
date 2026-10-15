#pragma once

#include <string>
#include <vector>
#include <imgui.h>

namespace parallax {

// Single-select dropdown
bool Dropdown(
    const char* label,
    const std::vector<std::string>& items,
    int* selected_index,
    const std::vector<std::string>& item_icons = {},
    float width = 0.0f
);

// Multi-select dropdown
bool MultiDropdown(
    const char* label,
    const std::vector<std::string>& items,
    std::vector<bool>& selected_states,
    const std::vector<std::string>& item_icons = {},
    float width = 0.0f
);

} // namespace parallax
