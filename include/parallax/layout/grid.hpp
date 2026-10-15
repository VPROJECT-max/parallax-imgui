#pragma once

#include <imgui.h>

namespace parallax::layout {

class Grid {
public:
    static void Begin(int columns = 2, float spacing = 12.0f);
    static void NextColumn();
    static void End();

    static float GetColumnWidth() noexcept;
};

class Groupbox {
public:
    static bool Begin(const char* title, const char* icon = nullptr, float height = 0.0f);
    static void End();
};

} // namespace parallax::layout
