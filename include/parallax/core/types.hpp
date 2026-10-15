#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <functional>
#include <imgui.h>

namespace parallax {

enum class NavSide : uint8_t {
    Left = 0,
    Right,
    Top,
    Bottom
};

enum class SubTabPlacement : uint8_t {
    TopHorizontal = 0,
    BottomHorizontal,
    NestedSidebar
};

enum class WindowSizeTier : uint8_t {
    Compact = 0,   // ~720x480
    Standard,      // ~900x600
    Expanded,      // ~1080x720
    Custom
};

enum class SpeedMultiplier : uint8_t {
    Fast = 0,      // 1.5x
    Normal,        // 1.0x
    Relaxed,       // 0.7x
    Off            // Instant snap / Reduced Motion
};

struct WindowTierDimensions {
    static constexpr ImVec2 Compact{720.0f, 480.0f};
    static constexpr ImVec2 Standard{900.0f, 600.0f};
    static constexpr ImVec2 Expanded{1080.0f, 720.0f};
    static constexpr ImVec2 MinBounds{640.0f, 440.0f};

    static constexpr ImVec2 GetDimensions(WindowSizeTier tier) noexcept {
        switch (tier) {
            case WindowSizeTier::Compact:  return Compact;
            case WindowSizeTier::Standard: return Standard;
            case WindowSizeTier::Expanded: return Expanded;
            case WindowSizeTier::Custom:   return Standard;
        }
        return Standard;
    }
};

struct UserProfile {
    std::string username{"User"};
    std::string subtitle{"Pro Account"};
    std::string icon{"user"};
    bool isOnline{true};
    ImVec4 statusColor{0.16f, 0.75f, 0.45f, 1.0f}; // Emerald active dot
};

using SearchQueryCallback = std::function<void(std::string_view query)>;

} // namespace parallax
