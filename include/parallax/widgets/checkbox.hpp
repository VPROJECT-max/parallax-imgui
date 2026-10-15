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

bool Toggle(
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

} // namespace parallax
