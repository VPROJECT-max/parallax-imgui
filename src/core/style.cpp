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

    style.ScrollbarSize     = 8.0f;
    style.GrabMinSize       = 10.0f;

    // Colors mapping to Dear ImGui
    colors[ImGuiCol_Text]                  = m_colors.TextPrimary;
    colors[ImGuiCol_TextDisabled]          = m_colors.TextMuted;
    colors[ImGuiCol_WindowBg]              = m_colors.WindowBg;
    colors[ImGuiCol_ChildBg]               = m_colors.CardBg;
    colors[ImGuiCol_PopupBg]               = m_colors.HeaderBg;
    colors[ImGuiCol_Border]                = m_colors.Border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);

    colors[ImGuiCol_FrameBg]               = ImVec4(0.10f, 0.11f, 0.14f, 1.0f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.14f, 0.15f, 0.19f, 1.0f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.18f, 0.19f, 0.24f, 1.0f);

    colors[ImGuiCol_TitleBg]               = m_colors.HeaderBg;
    colors[ImGuiCol_TitleBgActive]         = m_colors.HeaderBg;
    colors[ImGuiCol_TitleBgCollapsed]      = m_colors.HeaderBg;
    colors[ImGuiCol_MenuBarBg]             = m_colors.HeaderBg;

    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.24f, 0.26f, 0.32f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.32f, 0.34f, 0.42f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.40f, 0.42f, 0.52f, 1.0f);

    colors[ImGuiCol_CheckMark]             = m_colors.Accent;
    colors[ImGuiCol_SliderGrab]            = m_colors.Accent;
    colors[ImGuiCol_SliderGrabActive]      = m_colors.AccentActive;

    colors[ImGuiCol_Button]                = ImVec4(0.12f, 0.13f, 0.17f, 1.0f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.16f, 0.17f, 0.22f, 1.0f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.20f, 0.22f, 0.28f, 1.0f);

    colors[ImGuiCol_Header]                = m_colors.AccentMuted;
    colors[ImGuiCol_HeaderHovered]         = ImVec4(m_colors.Accent.x, m_colors.Accent.y, m_colors.Accent.z, 0.25f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(m_colors.Accent.x, m_colors.Accent.y, m_colors.Accent.z, 0.35f);

    colors[ImGuiCol_Separator]             = m_colors.Border;
    colors[ImGuiCol_SeparatorHovered]      = m_colors.BorderStrong;
    colors[ImGuiCol_SeparatorActive]       = m_colors.Accent;

    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(m_colors.Accent.x, m_colors.Accent.y, m_colors.Accent.z, 0.50f);
    colors[ImGuiCol_ResizeGripActive]      = m_colors.Accent;

    colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.11f, 0.14f, 1.0f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.15f, 0.16f, 0.21f, 1.0f);
    colors[ImGuiCol_TabActive]             = m_colors.CardBg;
    colors[ImGuiCol_TabUnfocused]          = ImVec4(0.10f, 0.11f, 0.14f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive]   = m_colors.CardBg;
}

} // namespace parallax
