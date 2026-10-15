#include "parallax/layout/presets.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"

namespace parallax {

const char* GetLayoutPresetName(LayoutPreset preset) {
    switch (preset) {
        case LayoutPreset::CompactTactical: return "Compact Tactical";
        case LayoutPreset::MinimalTabbed:    return "Minimal Tabbed";
        case LayoutPreset::ModernSidebar:
        default:                            return "Modern Sidebar";
    }
}

void SetLayoutPreset(LayoutPreset preset) {
    Context& ctx = Context::Get();
    ctx.CurrentLayoutPresetIndex = static_cast<int>(preset);

    auto& metrics = StyleManager::Get().Metrics();

    switch (preset) {
        case LayoutPreset::CompactTactical:
            ctx.CurrentNavSide = NavSide::Left;
            metrics.NavWidth = 68.0f;
            metrics.ItemSpacing = ImVec2(8.0f, 8.0f);
            metrics.FramePadding = ImVec2(8.0f, 5.0f);
            ctx.DefaultWindowSize = ImVec2(840.0f, 560.0f);
            break;

        case LayoutPreset::MinimalTabbed:
            ctx.CurrentNavSide = NavSide::Top;
            metrics.NavHeight = 52.0f;
            metrics.ItemSpacing = ImVec2(10.0f, 10.0f);
            metrics.FramePadding = ImVec2(10.0f, 6.0f);
            ctx.DefaultWindowSize = ImVec2(920.0f, 600.0f);
            break;

        case LayoutPreset::ModernSidebar:
        default:
            ctx.CurrentNavSide = NavSide::Left;
            metrics.NavWidth = 200.0f;
            metrics.ItemSpacing = ImVec2(10.0f, 10.0f);
            metrics.FramePadding = ImVec2(10.0f, 6.0f);
            ctx.DefaultWindowSize = ImVec2(960.0f, 620.0f);
            break;
    }
}

LayoutPreset GetLayoutPreset() {
    return static_cast<LayoutPreset>(Context::Get().CurrentLayoutPresetIndex);
}

} // namespace parallax
