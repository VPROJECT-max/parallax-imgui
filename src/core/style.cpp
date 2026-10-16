#include "parallax/core/style.hpp"
#include "parallax/anim/easing.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/core/context.hpp"
#include <algorithm>
#include <cmath>

namespace parallax {

namespace {

constexpr ImVec4 LerpVec4(const ImVec4& a, const ImVec4& b, float t) noexcept {
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    );
}

constexpr float RelativeLuminance(const ImVec4& c) noexcept {
    return 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
}

constexpr ImVec4 AdjustBrightness(const ImVec4& c, float delta) noexcept {
    return ImVec4(
        std::clamp(c.x + delta, 0.0f, 1.0f),
        std::clamp(c.y + delta, 0.0f, 1.0f),
        std::clamp(c.z + delta, 0.0f, 1.0f),
        c.w
    );
}

constexpr ImVec4 ScaleBrightness(const ImVec4& c, float factor) noexcept {
    return ImVec4(
        std::clamp(c.x * factor, 0.0f, 1.0f),
        std::clamp(c.y * factor, 0.0f, 1.0f),
        std::clamp(c.z * factor, 0.0f, 1.0f),
        c.w
    );
}

StyleColors LerpPalette(const StyleColors& a, const StyleColors& b, float t) noexcept {
    StyleColors out;
    out.WindowBg       = LerpVec4(a.WindowBg,       b.WindowBg,       t);
    out.HeaderBg       = LerpVec4(a.HeaderBg,       b.HeaderBg,       t);
    out.ContentBg      = LerpVec4(a.ContentBg,      b.ContentBg,      t);
    out.CardBg         = LerpVec4(a.CardBg,         b.CardBg,         t);
    out.CardHeaderBg   = LerpVec4(a.CardHeaderBg,   b.CardHeaderBg,   t);
    out.WidgetBg       = LerpVec4(a.WidgetBg,       b.WidgetBg,       t);
    out.CardHover      = LerpVec4(a.CardHover,      b.CardHover,      t);
    out.WidgetHover    = LerpVec4(a.WidgetHover,    b.WidgetHover,    t);
    out.WidgetActive   = LerpVec4(a.WidgetActive,   b.WidgetActive,   t);
    out.PopupBg        = LerpVec4(a.PopupBg,        b.PopupBg,        t);

    out.BorderSubtle   = LerpVec4(a.BorderSubtle,   b.BorderSubtle,   t);
    out.Border         = LerpVec4(a.Border,         b.Border,         t);
    out.BorderStrong   = LerpVec4(a.BorderStrong,   b.BorderStrong,   t);
    out.Shadow         = LerpVec4(a.Shadow,         b.Shadow,         t);
    out.OverlayDim     = LerpVec4(a.OverlayDim,     b.OverlayDim,     t);

    out.Accent         = LerpVec4(a.Accent,         b.Accent,         t);
    out.AccentHover    = LerpVec4(a.AccentHover,    b.AccentHover,    t);
    out.AccentActive   = LerpVec4(a.AccentActive,   b.AccentActive,   t);
    out.AccentMuted    = LerpVec4(a.AccentMuted,    b.AccentMuted,    t);

    out.TextPrimary    = LerpVec4(a.TextPrimary,    b.TextPrimary,    t);
    out.TextSecondary  = LerpVec4(a.TextSecondary,  b.TextSecondary,  t);
    out.TextMuted      = LerpVec4(a.TextMuted,      b.TextMuted,      t);
    out.TextOnAccent   = LerpVec4(a.TextOnAccent,   b.TextOnAccent,   t);

    out.ToggleThumbOff = LerpVec4(a.ToggleThumbOff, b.ToggleThumbOff, t);
    out.ToggleThumbOn  = LerpVec4(a.ToggleThumbOn,  b.ToggleThumbOn,  t);

    out.Success        = LerpVec4(a.Success,        b.Success,        t);
    out.Warning        = LerpVec4(a.Warning,        b.Warning,        t);
    out.Danger         = LerpVec4(a.Danger,         b.Danger,         t);
    return out;
}

} // namespace

StyleManager::StyleManager() {
    m_colors = BuildPalette(m_theme, m_accentPreset, m_customAccent);
    m_startColors = m_colors;
    m_targetColors = m_colors;
}

StyleManager& StyleManager::Get() noexcept {
    static StyleManager instance;
    return instance;
}

const char* StyleManager::GetThemeName(ThemeMode mode) noexcept {
    switch (mode) {
        case ThemeMode::Amoled: return "AMOLED (Black)";
        case ThemeMode::Dark:   return "Graphite (Dark)";
        case ThemeMode::Light:  return "Porcelain (Light)";
    }
    return "AMOLED (Black)";
}

const char* StyleManager::GetAccentPresetName(AccentPreset preset) noexcept {
    switch (preset) {
        case AccentPreset::Parallax:   return "Parallax Violet";
        case AccentPreset::Sage:       return "Muted Sage";
        case AccentPreset::Monochrome: return "Monochrome";
        case AccentPreset::Copper:     return "Desaturated Copper";
        case AccentPreset::SteelBlue:  return "Subtle Steel Blue";
        case AccentPreset::Custom:     return "Custom";
    }
    return "Parallax Violet";
}

ImVec4 StyleManager::GetAccentPreviewColor(AccentPreset preset, ThemeMode mode, const ImVec4& customAccent) noexcept {
    const bool light = (mode == ThemeMode::Light);
    switch (preset) {
        case AccentPreset::Parallax:
            return ImVec4(153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 1.00f); // #9998F6
        case AccentPreset::Sage:
            return light ? ImVec4(0.306f, 0.478f, 0.388f, 1.00f)   // #4E7A63
                         : ImVec4(0.471f, 0.631f, 0.545f, 1.00f);  // #78A18B
        case AccentPreset::Monochrome:
            return light ? ImVec4(0.243f, 0.271f, 0.322f, 1.00f)   // #3E4552
                         : ImVec4(0.557f, 0.592f, 0.651f, 1.00f);  // #8E97A6
        case AccentPreset::Copper:
            return light ? ImVec4(0.580f, 0.373f, 0.275f, 1.00f)   // #945F46
                         : ImVec4(0.710f, 0.514f, 0.416f, 1.00f);  // #B5836A
        case AccentPreset::SteelBlue:
            return light ? ImVec4(0.275f, 0.420f, 0.541f, 1.00f)   // #466B8A
                         : ImVec4(0.431f, 0.565f, 0.678f, 1.00f);  // #6E90AD
        case AccentPreset::Custom: {
            ImVec4 base = customAccent;
            base.w = 1.0f;
            float lum = RelativeLuminance(base);
            if (light && lum > 0.62f) {
                base = ScaleBrightness(base, 0.72f);
            } else if (!light && lum < 0.18f) {
                base = AdjustBrightness(base, 0.22f);
            }
            return base;
        }
    }
    return ImVec4(153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 1.00f);
}

StyleColors StyleManager::BuildPalette(ThemeMode mode, AccentPreset preset, const ImVec4& customAccent) noexcept {
    StyleColors c;

    if (mode == ThemeMode::Amoled) {
        // Pure Black AMOLED (#000000)
        c.WindowBg       = ImVec4(0.000f, 0.000f, 0.000f, 1.00f); // #000000
        c.HeaderBg       = ImVec4(0.020f, 0.020f, 0.027f, 1.00f); // #050507
        c.ContentBg      = ImVec4(0.039f, 0.043f, 0.055f, 1.00f); // #0A0B0E
        c.CardBg         = ImVec4(0.059f, 0.063f, 0.082f, 1.00f); // #0F1015
        c.CardHeaderBg   = ImVec4(0.078f, 0.086f, 0.110f, 1.00f); // #14161C
        c.WidgetBg       = ImVec4(0.078f, 0.086f, 0.110f, 1.00f); // #14161C
        c.CardHover      = ImVec4(0.086f, 0.098f, 0.133f, 1.00f); // #161922
        c.WidgetHover    = ImVec4(0.110f, 0.122f, 0.157f, 1.00f); // #1C1F28
        c.WidgetActive   = ImVec4(0.149f, 0.169f, 0.220f, 1.00f); // #262B38
        c.PopupBg        = ImVec4(0.043f, 0.047f, 0.063f, 0.98f); // #0B0C10

        c.BorderSubtle   = ImVec4(1.000f, 1.000f, 1.000f, 0.05f);
        c.Border         = ImVec4(1.000f, 1.000f, 1.000f, 0.08f);
        c.BorderStrong   = ImVec4(1.000f, 1.000f, 1.000f, 0.16f);
        c.Shadow         = ImVec4(0.000f, 0.000f, 0.000f, 0.65f);
        c.OverlayDim     = ImVec4(0.000f, 0.000f, 0.000f, 0.85f);

        c.TextPrimary    = ImVec4(0.961f, 0.965f, 0.980f, 1.00f); // #F5F6FA
        c.TextSecondary  = ImVec4(0.620f, 0.620f, 0.698f, 1.00f); // #9E9EB2
        c.TextMuted      = ImVec4(0.333f, 0.337f, 0.416f, 1.00f); // #55566A

        c.ToggleThumbOff = ImVec4(0.620f, 0.620f, 0.698f, 1.00f);
        c.ToggleThumbOn  = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);

        c.Success        = ImVec4(0.310f, 0.659f, 0.471f, 1.00f); // #4FA878
        c.Warning        = ImVec4(0.831f, 0.608f, 0.294f, 1.00f); // #D49B4B
        c.Danger         = ImVec4(0.788f, 0.337f, 0.337f, 1.00f); // #C95656
    } else if (mode == ThemeMode::Dark) {
        // Dark Theme — Graphite
        c.WindowBg       = ImVec4(0.051f, 0.059f, 0.071f, 1.00f); // #0D0F12
        c.HeaderBg       = ImVec4(0.067f, 0.078f, 0.094f, 1.00f); // #111418
        c.ContentBg      = ImVec4(0.078f, 0.094f, 0.114f, 1.00f); // #14181D
        c.CardBg         = ImVec4(0.102f, 0.122f, 0.149f, 1.00f); // #1A1F26
        c.CardHeaderBg   = ImVec4(0.118f, 0.141f, 0.173f, 1.00f); // #1E242C
        c.WidgetBg       = ImVec4(0.141f, 0.169f, 0.208f, 1.00f); // #242B35
        c.CardHover      = ImVec4(0.125f, 0.149f, 0.188f, 1.00f); // #202630
        c.WidgetHover    = ImVec4(0.173f, 0.208f, 0.259f, 1.00f); // #2C3542
        c.WidgetActive   = ImVec4(0.204f, 0.247f, 0.306f, 1.00f); // #343F4E
        c.PopupBg        = ImVec4(0.094f, 0.110f, 0.137f, 0.98f); // #181C23

        c.BorderSubtle   = ImVec4(1.000f, 1.000f, 1.000f, 0.05f);
        c.Border         = ImVec4(1.000f, 1.000f, 1.000f, 0.08f);
        c.BorderStrong   = ImVec4(1.000f, 1.000f, 1.000f, 0.16f);
        c.Shadow         = ImVec4(0.000f, 0.000f, 0.000f, 0.48f);
        c.OverlayDim     = ImVec4(0.024f, 0.031f, 0.043f, 0.72f);

        c.TextPrimary    = ImVec4(0.925f, 0.937f, 0.957f, 1.00f); // #ECEFF4
        c.TextSecondary  = ImVec4(0.604f, 0.639f, 0.698f, 1.00f); // #9AA3B2
        c.TextMuted      = ImVec4(0.388f, 0.427f, 0.494f, 1.00f); // #636D7E

        c.ToggleThumbOff = ImVec4(0.604f, 0.639f, 0.698f, 1.00f);
        c.ToggleThumbOn  = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);

        c.Success        = ImVec4(0.310f, 0.659f, 0.471f, 1.00f); // #4FA878
        c.Warning        = ImVec4(0.831f, 0.608f, 0.294f, 1.00f); // #D49B4B
        c.Danger         = ImVec4(0.788f, 0.337f, 0.337f, 1.00f); // #C95656
    } else {
        // Light Theme — Porcelain
        c.WindowBg       = ImVec4(0.918f, 0.910f, 0.882f, 1.00f); // #EAE8E1
        c.HeaderBg       = ImVec4(0.894f, 0.882f, 0.851f, 1.00f); // #E4E1D9
        c.ContentBg      = ImVec4(0.953f, 0.945f, 0.925f, 1.00f); // #F3F1EC
        c.CardBg         = ImVec4(1.000f, 1.000f, 1.000f, 1.00f); // #FFFFFF
        c.CardHeaderBg   = ImVec4(0.973f, 0.969f, 0.957f, 1.00f); // #F8F7F4
        c.WidgetBg       = ImVec4(0.918f, 0.906f, 0.878f, 1.00f); // #EAE7E0
        c.CardHover      = ImVec4(0.957f, 0.949f, 0.929f, 1.00f); // #F4F2ED
        c.WidgetHover    = ImVec4(0.875f, 0.859f, 0.824f, 1.00f); // #DFDBD2
        c.WidgetActive   = ImVec4(0.827f, 0.808f, 0.765f, 1.00f); // #D3CEC3
        c.PopupBg        = ImVec4(1.000f, 1.000f, 1.000f, 0.99f); // #FFFFFF

        c.BorderSubtle   = ImVec4(0.094f, 0.110f, 0.133f, 0.06f);
        c.Border         = ImVec4(0.094f, 0.110f, 0.133f, 0.11f);
        c.BorderStrong   = ImVec4(0.094f, 0.110f, 0.133f, 0.22f);
        c.Shadow         = ImVec4(0.071f, 0.086f, 0.110f, 0.10f);
        c.OverlayDim     = ImVec4(0.094f, 0.110f, 0.133f, 0.38f);

        c.TextPrimary    = ImVec4(0.094f, 0.106f, 0.125f, 1.00f); // #181B20
        c.TextSecondary  = ImVec4(0.322f, 0.353f, 0.400f, 1.00f); // #525A66
        c.TextMuted      = ImVec4(0.518f, 0.553f, 0.604f, 1.00f); // #848D9A

        c.ToggleThumbOff = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);
        c.ToggleThumbOn  = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);

        c.Success        = ImVec4(0.180f, 0.522f, 0.333f, 1.00f); // #2E8555
        c.Warning        = ImVec4(0.722f, 0.459f, 0.078f, 1.00f); // #B87514
        c.Danger         = ImVec4(0.722f, 0.231f, 0.231f, 1.00f); // #B83B3B
    }

    const ImVec4 baseAccent = GetAccentPreviewColor(preset, mode, customAccent);
    c.Accent = baseAccent;
    c.AccentHover = AdjustBrightness(baseAccent, 0.075f);
    c.AccentActive = AdjustBrightness(baseAccent, -0.075f);
    c.AccentMuted = ImVec4(baseAccent.x, baseAccent.y, baseAccent.z, (mode == ThemeMode::Light) ? 0.15f : 0.18f);

    const float accentLum = RelativeLuminance(baseAccent);
    if (accentLum > 0.68f) {
        c.TextOnAccent = ImVec4(0.078f, 0.094f, 0.114f, 1.00f);
    } else {
        c.TextOnAccent = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);
    }

    return c;
}

void StyleManager::StartOrSnapTransition(const StyleColors& newTarget, bool animate) {
    m_targetColors = newTarget;

    bool reducedMotion = false;
    if (Context::Get().IsInitialized()) {
        reducedMotion = (Context::Get().Anim().GetSpeedMultiplier() == SpeedMultiplier::Off);
    }

    if (!animate || reducedMotion || ImGui::GetCurrentContext() == nullptr) {
        m_colors = m_targetColors;
        m_startColors = m_targetColors;
        m_transitionElapsed = 0.0f;
        m_transitionActive = false;
        if (ImGui::GetCurrentContext() != nullptr) {
            ApplyToImGui();
        }
        return;
    }

    m_startColors = m_colors;
    m_transitionElapsed = 0.0f;
    m_transitionActive = true;
}

void StyleManager::SetTheme(ThemeMode mode, bool animate) {
    if (m_theme == mode && !m_transitionActive) {
        return;
    }
    m_theme = mode;
    StartOrSnapTransition(BuildPalette(m_theme, m_accentPreset, m_customAccent), animate);
}

void StyleManager::ToggleTheme(bool animate) {
    if (m_theme == ThemeMode::Amoled) {
        SetTheme(ThemeMode::Dark, animate);
    } else if (m_theme == ThemeMode::Dark) {
        SetTheme(ThemeMode::Light, animate);
    } else {
        SetTheme(ThemeMode::Amoled, animate);
    }
}

void StyleManager::SetAccentPreset(AccentPreset preset, bool animate) {
    m_accentPreset = preset;
    StartOrSnapTransition(BuildPalette(m_theme, m_accentPreset, m_customAccent), animate);
}

void StyleManager::SetCustomAccent(const ImVec4& color, bool animate) {
    m_customAccent = ImVec4(color.x, color.y, color.z, 1.0f);
    m_accentPreset = AccentPreset::Custom;
    StartOrSnapTransition(BuildPalette(m_theme, m_accentPreset, m_customAccent), animate);
}

void StyleManager::UpdateTransition(float dt) {
    if (!m_transitionActive) {
        return;
    }

    float duration = std::max(m_metrics.ThemeTransitionDuration, 0.01f);
    m_transitionElapsed += std::clamp(dt, 0.0f, 0.1f);
    float progress = std::clamp(m_transitionElapsed / duration, 0.0f, 1.0f);
    float eased = anim::CubicBezier::GetPreset(anim::EasingPreset::EaseOutQuint).Solve(progress);

    m_colors = LerpPalette(m_startColors, m_targetColors, eased);

    if (progress >= 1.0f) {
        m_colors = m_targetColors;
        m_transitionActive = false;
    }

    if (ImGui::GetCurrentContext() != nullptr) {
        ApplyToImGui();
    }
}

void StyleManager::ApplyToImGui() {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Metrics & Roundings
    style.WindowRounding    = m_metrics.WindowRounding;
    style.ChildRounding     = m_metrics.ChildRounding;
    style.FrameRounding     = m_metrics.FrameRounding;
    style.PopupRounding     = m_metrics.PopupRounding;
    style.ScrollbarRounding = m_metrics.ScrollbarRounding;
    style.GrabRounding      = m_metrics.GrabRounding;
    style.TabRounding       = m_metrics.TabRounding;

    style.WindowPadding     = m_metrics.WindowPadding;
    style.FramePadding      = m_metrics.FramePadding;
    style.ItemSpacing       = m_metrics.ItemSpacing;
    style.ItemInnerSpacing  = m_metrics.ItemInnerSpacing;

    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    // Sleek 6px scrollbar
    style.ScrollbarSize     = 6.0f;
    style.GrabMinSize       = 14.0f;

    // Colors mapping to Dear ImGui based on the 5-layer hierarchy
    colors[ImGuiCol_Text]                  = m_colors.TextPrimary;
    colors[ImGuiCol_TextDisabled]          = m_colors.TextMuted;
    colors[ImGuiCol_WindowBg]              = m_colors.WindowBg;
    colors[ImGuiCol_ChildBg]               = m_colors.ContentBg;
    colors[ImGuiCol_PopupBg]               = m_colors.PopupBg;
    colors[ImGuiCol_Border]                = m_colors.Border;
    colors[ImGuiCol_BorderShadow]          = m_colors.Shadow;

    colors[ImGuiCol_FrameBg]               = m_colors.WidgetBg;
    colors[ImGuiCol_FrameBgHovered]        = m_colors.WidgetHover;
    colors[ImGuiCol_FrameBgActive]         = m_colors.WidgetActive;

    colors[ImGuiCol_TitleBg]               = m_colors.HeaderBg;
    colors[ImGuiCol_TitleBgActive]         = m_colors.HeaderBg;
    colors[ImGuiCol_TitleBgCollapsed]      = m_colors.HeaderBg;
    colors[ImGuiCol_MenuBarBg]             = m_colors.HeaderBg;

    // Theme-aware floating scrollbar
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(
        m_colors.TextMuted.x, m_colors.TextMuted.y, m_colors.TextMuted.z, 0.45f
    );
    colors[ImGuiCol_ScrollbarGrabHovered]  = m_colors.Accent;
    colors[ImGuiCol_ScrollbarGrabActive]   = m_colors.AccentActive;

    colors[ImGuiCol_CheckMark]             = m_colors.Accent;
    colors[ImGuiCol_SliderGrab]            = m_colors.Accent;
    colors[ImGuiCol_SliderGrabActive]      = m_colors.AccentActive;

    colors[ImGuiCol_Button]                = m_colors.WidgetBg;
    colors[ImGuiCol_ButtonHovered]         = m_colors.WidgetHover;
    colors[ImGuiCol_ButtonActive]          = m_colors.AccentMuted;

    colors[ImGuiCol_Header]                = m_colors.AccentMuted;
    colors[ImGuiCol_HeaderHovered]         = ImVec4(m_colors.Accent.x, m_colors.Accent.y, m_colors.Accent.z, 0.25f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(m_colors.Accent.x, m_colors.Accent.y, m_colors.Accent.z, 0.35f);

    colors[ImGuiCol_Separator]             = m_colors.Border;
    colors[ImGuiCol_SeparatorHovered]      = m_colors.BorderStrong;
    colors[ImGuiCol_SeparatorActive]       = m_colors.Accent;

    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[ImGuiCol_Tab]                   = m_colors.HeaderBg;
    colors[ImGuiCol_TabHovered]            = m_colors.WidgetHover;
    colors[ImGuiCol_TabActive]             = m_colors.CardBg;
    colors[ImGuiCol_TabUnfocused]          = m_colors.HeaderBg;
    colors[ImGuiCol_TabUnfocusedActive]   = m_colors.CardBg;
}

} // namespace parallax
