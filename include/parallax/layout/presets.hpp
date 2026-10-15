#pragma once

#include "parallax/core/types.hpp"
#include <string>

namespace parallax {

enum class LayoutPreset {
    ModernSidebar,   // Spacious left sidebar (200px) with icons & text labels, top sub-tabs, search bar header
    CompactTactical, // Narrow icon-only dock (64px), high information density, tight spacing, compact tactical cards
    MinimalTabbed    // Sleek horizontal top navigation tabs (0px sidebar), minimal clean layout, floating watermark
};

const char* GetLayoutPresetName(LayoutPreset preset);
void SetLayoutPreset(LayoutPreset preset);
LayoutPreset GetLayoutPreset();

} // namespace parallax
