#pragma once

#include <imgui.h>

namespace parallax {

struct StyleColors {
    ImVec4 WindowBg{0.047f, 0.051f, 0.067f, 0.98f};       // Deep Slate/Zinc #0C0D11
    ImVec4 CardBg{0.078f, 0.086f, 0.114f, 1.00f};         // Surface Card #14161D
    ImVec4 CardHover{0.098f, 0.106f, 0.137f, 1.00f};      // Hovered Card #191B23
    ImVec4 HeaderBg{0.063f, 0.067f, 0.086f, 1.00f};       // Header / Nav #101116
    ImVec4 Border{1.000f, 1.000f, 1.000f, 0.07f};         // 1px Subtle Border rgba(255,255,255,0.07)
    ImVec4 BorderStrong{1.000f, 1.000f, 1.000f, 0.14f};   // Active/Hovered Border

    ImVec4 Accent{0.388f, 0.400f, 0.945f, 1.00f};         // Electric Indigo #6366F1
    ImVec4 AccentHover{0.486f, 0.498f, 0.980f, 1.00f};    // #7C7FFB
    ImVec4 AccentActive{0.310f, 0.322f, 0.855f, 1.00f};   // #4F52DA
    ImVec4 AccentMuted{0.388f, 0.400f, 0.945f, 0.15f};    // 15% Accent Tint

    ImVec4 TextPrimary{0.953f, 0.957f, 0.965f, 1.00f};    // Zinc-100 #F3F4F6
    ImVec4 TextSecondary{0.612f, 0.639f, 0.686f, 1.00f};  // Zinc-400 #9CA3AF
    ImVec4 TextMuted{0.420f, 0.447f, 0.502f, 1.00f};      // Zinc-500 #6B7280

    ImVec4 Success{0.133f, 0.773f, 0.369f, 1.00f};        // Emerald #22C55E
    ImVec4 Warning{0.961f, 0.620f, 0.043f, 1.00f};        // Amber #F59E0B
    ImVec4 Danger{0.937f, 0.267f, 0.267f, 1.00f};         // Crimson #EF4444
};

struct StyleMetrics {
    float WindowRounding{8.0f};
    float ChildRounding{8.0f};
    float FrameRounding{6.0f};
    float PopupRounding{6.0f};
    float ScrollbarRounding{4.0f};
    float GrabRounding{4.0f};
    float TabRounding{6.0f};

    ImVec2 WindowPadding{16.0f, 16.0f};
    ImVec2 FramePadding{10.0f, 6.0f};
    ImVec2 ItemSpacing{8.0f, 8.0f};
    ImVec2 ItemInnerSpacing{6.0f, 6.0f};

    float NavWidth{200.0f};
    float NavHeight{56.0f};
    float SubTabHeight{36.0f};
    float SearchBarWidth{220.0f};
    float ProfileCardHeight{56.0f};
};

class StyleManager {
public:
    static StyleManager& Get() noexcept;

    const StyleColors& Colors() const noexcept { return m_colors; }
    StyleColors& Colors() noexcept { return m_colors; }

    const StyleMetrics& Metrics() const noexcept { return m_metrics; }
    StyleMetrics& Metrics() noexcept { return m_metrics; }

    void ApplyToImGui();

private:
    StyleManager();
    StyleColors m_colors;
    StyleMetrics m_metrics;
};

} // namespace parallax
