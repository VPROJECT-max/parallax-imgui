#include "parallax/core/style.hpp"

namespace parallax {

StyleManager::StyleManager() {
    // Default palette and metrics initialized via member initializers
}

StyleManager& StyleManager::Get() noexcept {
    static StyleManager instance;
    return instance;
}

void StyleManager::ApplyToImGui() {
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

    // Sleek 5px scrollbar with 0px margin
    style.ScrollbarSize     = 5.0f;
    style.GrabMinSize       = 14.0f;

    // Colors mapping to Dear ImGui based on the 5-layer hierarchy
    colors[ImGuiCol_Text]                  = m_colors.TextPrimary;
    colors[ImGuiCol_TextDisabled]          = m_colors.TextMuted;
    colors[ImGuiCol_WindowBg]              = m_colors.WindowBg;    // Layer 1 (#0C0D12)
    colors[ImGuiCol_ChildBg]               = m_colors.ContentBg;   // Layer 3 (#141722)
    colors[ImGuiCol_PopupBg]               = m_colors.HeaderBg;    // Layer 2 (#11131B)
    colors[ImGuiCol_Border]                = m_colors.Border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);

    colors[ImGuiCol_FrameBg]               = m_colors.WidgetBg;    // Layer 5 (#222638)
    colors[ImGuiCol_FrameBgHovered]        = m_colors.WidgetHover;
    colors[ImGuiCol_FrameBgActive]         = m_colors.CardHover;

    colors[ImGuiCol_TitleBg]               = m_colors.HeaderBg;
    colors[ImGuiCol_TitleBgActive]         = m_colors.HeaderBg;
    colors[ImGuiCol_TitleBgCollapsed]      = m_colors.HeaderBg;
    colors[ImGuiCol_MenuBarBg]             = m_colors.HeaderBg;

    // Sleek floating scrollbar
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.24f, 0.27f, 0.38f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = m_colors.Accent;
    colors[ImGuiCol_ScrollbarGrabActive]   = m_colors.AccentHover;

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
