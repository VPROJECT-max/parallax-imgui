#pragma once

#include "parallax/core/types.hpp"
#include <imgui.h>

namespace parallax {

struct StyleColors {
    // 5 Strongly Separated Visual Layers:
    ImVec4 WindowBg{0.051f, 0.059f, 0.071f, 1.00f};       // Layer 1: Application Surface (#0D0F12)
    ImVec4 HeaderBg{0.067f, 0.078f, 0.094f, 1.00f};       // Layer 2: Navigation Surface (#111418)
    ImVec4 ContentBg{0.078f, 0.094f, 0.114f, 1.00f};      // Layer 3: Workspace Surface (#14181D)
    ImVec4 CardBg{0.102f, 0.122f, 0.149f, 1.00f};         // Layer 4: Content Sections / Cards (#1A1F26)
    ImVec4 CardHeaderBg{0.118f, 0.141f, 0.173f, 1.00f};   // Layer 4b: Elevated Card Header (#1E242C)
    ImVec4 WidgetBg{0.141f, 0.169f, 0.208f, 1.00f};       // Layer 5: Interactive Control Surface (#242B35)
    ImVec4 CardHover{0.125f, 0.149f, 0.188f, 1.00f};      // Hovered Card/Row (#202630)
    ImVec4 WidgetHover{0.173f, 0.208f, 0.259f, 1.00f};    // Hovered Control (#2C3542)
    ImVec4 WidgetActive{0.204f, 0.247f, 0.306f, 1.00f};   // Pressed/Active Control (#343F4E)
    ImVec4 PopupBg{0.094f, 0.110f, 0.137f, 0.98f};        // Floating Popup / Dropdown Surface (#181C23)

    // Borders, Separators & Elevation:
    ImVec4 BorderSubtle{1.000f, 1.000f, 1.000f, 0.05f};   // Hairline internal divider
    ImVec4 Border{1.000f, 1.000f, 1.000f, 0.08f};         // 1px Structural Border
    ImVec4 BorderStrong{1.000f, 1.000f, 1.000f, 0.16f};   // Active/Hovered/Focused Border
    ImVec4 Shadow{0.000f, 0.000f, 0.000f, 0.48f};         // Elevation Drop Shadow
    ImVec4 OverlayDim{0.024f, 0.031f, 0.043f, 0.72f};     // Modal Backdrop Dimming

    // Primary Brand Accent System (Default: #9998F6 / RGB 153, 152, 246):
    ImVec4 Accent{153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 1.00f};         // Primary Accent (#9998F6)
    ImVec4 AccentHover{175.0f / 255.0f, 174.0f / 255.0f, 250.0f / 255.0f, 1.00f};    // Hovered Accent
    ImVec4 AccentActive{135.0f / 255.0f, 134.0f / 255.0f, 235.0f / 255.0f, 1.00f};   // Active Accent
    ImVec4 AccentMuted{153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 0.18f};    // Translucent Accent Fill

    // Typography Hierarchy:
    ImVec4 TextPrimary{0.925f, 0.937f, 0.957f, 1.00f};    // Primary Readable Text (#ECEFF4)
    ImVec4 TextSecondary{0.604f, 0.639f, 0.698f, 1.00f};  // Secondary Label Text (#9AA3B2)
    ImVec4 TextMuted{0.388f, 0.427f, 0.494f, 1.00f};      // Muted / Disabled Text (#636D7E)
    ImVec4 TextOnAccent{1.000f, 1.000f, 1.000f, 1.00f};   // High-Contrast Text/Icon on Solid Accent

    // Control Specific Tokens:
    ImVec4 ToggleThumbOff{0.604f, 0.639f, 0.698f, 1.00f}; // Inactive Toggle Thumb
    ImVec4 ToggleThumbOn{1.000f, 1.000f, 1.000f, 1.00f};  // Active Toggle Thumb

    // Semantic Status Colors:
    ImVec4 Success{0.310f, 0.659f, 0.471f, 1.00f};        // Botanical Emerald (#4FA878)
    ImVec4 Warning{0.831f, 0.608f, 0.294f, 1.00f};        // Warm Ochre (#D49B4B)
    ImVec4 Danger{0.788f, 0.337f, 0.337f, 1.00f};         // Terracotta Crimson (#C95656)
};

struct StyleMetrics {
    // Corner Rounding Tokens:
    float WindowRounding{10.0f};
    float ChildRounding{8.0f};
    float FrameRounding{6.0f};
    float PopupRounding{8.0f};
    float ScrollbarRounding{4.0f};
    float GrabRounding{5.0f};
    float TabRounding{6.0f};

    // Spacing Scale Tokens:
    float SpacingXS{4.0f};
    float SpacingSM{8.0f};
    float SpacingMD{12.0f};
    float SpacingLG{16.0f};
    float SpacingXL{24.0f};

    ImVec2 WindowPadding{16.0f, 16.0f};
    ImVec2 FramePadding{10.0f, 6.0f};
    ImVec2 ItemSpacing{8.0f, 8.0f};
    ImVec2 ItemInnerSpacing{6.0f, 6.0f};

    // Card & Control Sizing Tokens:
    float CardPadding{14.0f};
    float CardSpacing{14.0f};
    float CardHeaderHeight{34.0f};
    float ControlHeight{30.0f};
    float ControlHeightCompact{24.0f};
    float RowHeight{30.0f};

    // Navigation & Shell Metrics:
    float NavWidth{204.0f};
    float NavHeight{56.0f};
    float SubTabHeight{36.0f};
    float SearchBarWidth{220.0f};
    float ProfileCardHeight{58.0f};

    // Typography Scale Tokens:
    float FontScaleSmall{0.86f};
    float FontScaleBody{1.00f};
    float FontScaleSubheading{1.08f};
    float FontScaleHeading{1.22f};

    // Animation Duration Tokens (seconds):
    float AnimFast{0.12f};
    float AnimNormal{0.20f};
    float AnimSlow{0.30f};
    float ThemeTransitionDuration{0.24f};
};

class StyleManager {
public:
    static StyleManager& Get() noexcept;

    const StyleColors& Colors() const noexcept { return m_colors; }
    StyleColors& Colors() noexcept { return m_colors; }

    const StyleMetrics& Metrics() const noexcept { return m_metrics; }
    StyleMetrics& Metrics() noexcept { return m_metrics; }

    // Theme Management (Amoled = Pure Black, Dark = Graphite, Light = Porcelain)
    void SetTheme(ThemeMode mode, bool animate = true);
    ThemeMode GetTheme() const noexcept { return m_theme; }
    void ToggleTheme(bool animate = true);
    bool IsLightTheme() const noexcept { return m_theme == ThemeMode::Light; }
    bool IsAmoledTheme() const noexcept { return m_theme == ThemeMode::Amoled; }

    // Accent Management (Parallax #9998F6, Sage, Monochrome, Copper, SteelBlue, Custom)
    void SetAccentPreset(AccentPreset preset, bool animate = true);
    AccentPreset GetAccentPreset() const noexcept { return m_accentPreset; }

    void SetCustomAccent(const ImVec4& color, bool animate = true);
    ImVec4 GetCustomAccent() const noexcept { return m_customAccent; }

    // Smooth per-frame theme & accent color interpolation
    void UpdateTransition(float dt);
    bool IsTransitioning() const noexcept { return m_transitionActive; }

    // Build complete palette for given theme & accent
    static StyleColors BuildPalette(ThemeMode mode, AccentPreset preset, const ImVec4& customAccent) noexcept;
    static const char* GetThemeName(ThemeMode mode) noexcept;
    static const char* GetAccentPresetName(AccentPreset preset) noexcept;
    static ImVec4 GetAccentPreviewColor(AccentPreset preset, ThemeMode mode, const ImVec4& customAccent = ImVec4(153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 1.0f)) noexcept;

    void ApplyToImGui();

private:
    StyleManager();

    void StartOrSnapTransition(const StyleColors& newTarget, bool animate);

    ThemeMode m_theme{ThemeMode::Amoled};
    AccentPreset m_accentPreset{AccentPreset::Parallax};
    ImVec4 m_customAccent{153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 1.00f};

    StyleColors m_colors;
    StyleColors m_startColors;
    StyleColors m_targetColors;
    StyleMetrics m_metrics;

    float m_transitionElapsed{0.0f};
    bool m_transitionActive{false};
};

} // namespace parallax
