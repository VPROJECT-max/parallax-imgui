#pragma once

#include "parallax/core/types.hpp"
#include <imgui.h>

namespace parallax::layout {

class NavBar {
public:
    static bool Begin(NavSide side);
    static void End();

    static bool TabItem(const char* icon, const char* label, bool selected, const char* badge = nullptr);

    static bool IsExpanded();
    static void SetExpanded(bool expanded);
    static void ToggleExpanded();
    static float GetNavWidth();
};

} // namespace parallax::layout
