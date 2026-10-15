#pragma once

#include <string>
#include <imgui.h>

namespace parallax {

// Custom Parallax Text Input with prefix icon, placeholder, animated focus ring, and clear button
bool InputText(
    const char* label,
    std::string& text,
    const char* icon = nullptr,
    const char* placeholder = nullptr,
    float width = 0.0f,
    bool isPassword = false
);

bool InputText(
    const char* label,
    char* buf,
    size_t bufSize,
    const char* icon = nullptr,
    const char* placeholder = nullptr,
    float width = 0.0f,
    bool isPassword = false
);

} // namespace parallax
