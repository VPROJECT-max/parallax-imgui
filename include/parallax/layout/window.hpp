#pragma once

#include "parallax/core/types.hpp"
#include <imgui.h>

namespace parallax::layout {

class WindowShell {
public:
    static bool Begin(const char* title, bool* p_open = nullptr, ImGuiWindowFlags extraFlags = 0);
    static void End();

    static ImVec2 GetContentPosition() noexcept;
    static ImVec2 GetContentSize() noexcept;
};

} // namespace parallax::layout
