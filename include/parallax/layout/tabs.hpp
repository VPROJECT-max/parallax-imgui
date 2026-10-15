#pragma once

#include "parallax/core/types.hpp"
#include <imgui.h>

namespace parallax::layout {

class SubTabs {
public:
    static bool Begin(SubTabPlacement placement);
    static void End();

    static bool Item(const char* label, bool selected);
};

} // namespace parallax::layout
