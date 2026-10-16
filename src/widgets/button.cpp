#include "parallax/widgets/button.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include <imgui_internal.h>
#include <algorithm>

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

} // namespace

bool ButtonEx(const char* label, const ButtonOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& gCtx = *GImGui;
    const ImGuiStyle& style = gCtx.Style;
    const ImGuiID id = window->GetID(label);

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();

    PushFont(FontWeight::Medium);

    const ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
    const bool hasText = (labelSize.x > 0.5f);
    const bool hasIcon = (options.icon && options.icon[0] != '\0');
    const float iconSize = hasIcon ? ((options.compact ? 14.0f : 16.0f) * dpi) : 0.0f;
    const float iconSpacing = (hasIcon && hasText) ? (7.0f * dpi) : 0.0f;

    const float defaultHeight = (options.compact ? 26.0f : 34.0f) * dpi;
    const float padX = (options.compact ? 10.0f : 14.0f) * dpi;
    const float minWidth = hasText
        ? (labelSize.x + iconSize + iconSpacing + padX * 2.0f)
        : defaultHeight;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(options.size, minWidth, defaultHeight);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) {
        PopFont();
        return false;
    }

    bool hovered = false;
    bool held = false;
    bool pressed = false;
    if (!options.disabled) {
        pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    }

    const float targetHover = (hovered && !options.disabled) ? 1.0f : 0.0f;
    const float animHover = ctx.Anim().SpringFloat(id + 0x11000, targetHover, {160.0f, 20.0f, 1.0f, 0.001f});

    const float targetActive = (held && !options.disabled) ? 1.0f : 0.0f;
    const float animActive = ctx.Anim().SpringFloat(id + 0x22000, targetActive, {200.0f, 22.0f, 1.0f, 0.001f});

    ImDrawList* drawList = window->DrawList;
    const float rounding = (options.compact ? (metrics.FrameRounding - 1.0f) : metrics.FrameRounding) * dpi;

    // Tactile vertical press micro-displacement
    const float pressOffsetY = animActive * 1.0f * dpi;
    const ImVec2 drawMin(bb.Min.x, bb.Min.y + pressOffsetY * 0.5f);
    const ImVec2 drawMax(bb.Max.x, bb.Max.y + pressOffsetY * 0.5f);

    ImVec4 bgCol{};
    ImVec4 borderCol{};
    ImVec4 textCol{};

    switch (options.variant) {
        case ButtonVariant::Primary: {
            ImVec4 base = LerpVec4(colors.Accent, colors.AccentHover, animHover);
            bgCol = LerpVec4(base, colors.AccentActive, animActive);
            borderCol = LerpVec4(colors.AccentActive, colors.AccentHover, animHover);
            borderCol.w = 0.65f;
            textCol = colors.TextOnAccent;
            break;
        }
        case ButtonVariant::Danger: {
            ImVec4 restBg = LerpVec4(colors.WidgetBg, colors.Danger, 0.16f);
            ImVec4 hoverBg = LerpVec4(colors.WidgetHover, colors.Danger, 0.28f);
            ImVec4 activeBg = LerpVec4(colors.WidgetActive, colors.Danger, 0.38f);
            bgCol = LerpVec4(LerpVec4(restBg, hoverBg, animHover), activeBg, animActive);
            borderCol = colors.Danger;
            borderCol.w = 0.38f + 0.42f * animHover;
            textCol = colors.Danger;
            break;
        }
        case ButtonVariant::Ghost: {
            bgCol = colors.WidgetHover;
            bgCol.w = 0.75f * animHover + 0.20f * animActive;
            borderCol = colors.Border;
            borderCol.w *= animHover;
            textCol = LerpVec4(colors.TextSecondary, colors.TextPrimary, animHover);
            break;
        }
        case ButtonVariant::Secondary:
        default: {
            ImVec4 base = LerpVec4(colors.WidgetBg, colors.WidgetHover, animHover);
            bgCol = LerpVec4(base, colors.WidgetActive, animActive);
            borderCol = LerpVec4(colors.Border, colors.BorderStrong, animHover);
            textCol = colors.TextPrimary;
            break;
        }
    }

    if (options.disabled) {
        bgCol.w *= 0.45f;
        borderCol.w *= 0.40f;
        textCol = colors.TextMuted;
    }

    // Subtle elevation shadow for Primary & Secondary buttons
    if (!options.disabled && options.variant != ButtonVariant::Ghost && animActive < 0.8f) {
        ImVec4 shadowCol = colors.Shadow;
        shadowCol.w *= (options.variant == ButtonVariant::Primary ? 0.35f : 0.20f) * (1.0f - animActive);
        drawList->AddRectFilled(
            ImVec2(drawMin.x, drawMin.y + 1.5f * dpi),
            ImVec2(drawMax.x, drawMax.y + 1.5f * dpi),
            ImColor(shadowCol),
            rounding
        );
    }

    // Subtle outer accent halo on Primary hover
    if (!options.disabled && options.variant == ButtonVariant::Primary && animHover > 0.02f) {
        ImVec4 glow = colors.AccentMuted;
        glow.w *= 0.65f * animHover;
        drawList->AddRect(
            ImVec2(drawMin.x - 1.5f * dpi, drawMin.y - 1.5f * dpi),
            ImVec2(drawMax.x + 1.5f * dpi, drawMax.y + 1.5f * dpi),
            ImColor(glow),
            rounding + 1.5f * dpi,
            0,
            1.5f * dpi
        );
    }

    if (bgCol.w > 0.005f) {
        drawList->AddRectFilled(drawMin, drawMax, ImColor(bgCol), rounding);
    }
    if (borderCol.w > 0.005f) {
        drawList->AddRect(drawMin, drawMax, ImColor(borderCol), rounding, 0, 1.0f);
    }

    // Centered content layout
    const float totalContentWidth = iconSize + iconSpacing + labelSize.x;
    float startX = drawMin.x + (size.x - totalContentWidth) * 0.5f;
    if (hasText) {
        startX = std::max(startX, drawMin.x + 8.0f * dpi);
    }
    const float centerY = (drawMin.y + drawMax.y) * 0.5f;

    if (hasIcon) {
        const ImVec2 iconPos(startX, centerY - (iconSize * 0.5f));
        ctx.Icons().DrawIcon(drawList, options.icon, iconPos, ImVec2(iconSize, iconSize), textCol);
        startX += iconSize + iconSpacing;
    }

    const char* textEnd = ImGui::FindRenderedTextEnd(label);
    if (textEnd > label && hasText) {
        const ImVec2 textPos(startX, centerY - (labelSize.y * 0.5f));
        drawList->AddText(nullptr, 0.0f, textPos, ImColor(textCol), label, textEnd);
    }

    if (options.tooltip && options.tooltip[0] != '\0' && hovered) {
        ImGui::SetTooltip("%s", options.tooltip);
    }

    PopFont();
    return pressed;
}

bool Button(const char* label, const char* icon, ButtonVariant variant, const ImVec2& sizeArg) {
    ButtonOptions opt;
    opt.icon = icon;
    opt.variant = variant;
    opt.size = sizeArg;
    return ButtonEx(label, opt);
}

bool ButtonCompact(const char* label, const char* icon, ButtonVariant variant, const ImVec2& sizeArg) {
    ButtonOptions opt;
    opt.icon = icon;
    opt.variant = variant;
    opt.size = sizeArg;
    opt.compact = true;
    return ButtonEx(label, opt);
}

bool IconButton(const char* idLabel, const char* icon, ButtonVariant variant, float dimension, const char* tooltip) {
    const float dpi = Context::Get().GetDpiScale();
    const float dim = (dimension > 0.0f) ? dimension : (28.0f * dpi);
    ButtonOptions opt;
    opt.icon = icon;
    opt.variant = variant;
    opt.size = ImVec2(dim, dim);
    opt.compact = (dim <= 28.0f * dpi);
    opt.tooltip = tooltip;

    // Ensure only the ##id part is used so no text is rendered
    char hiddenLabel[96];
    if (idLabel && idLabel[0] == '#' && idLabel[1] == '#') {
        snprintf(hiddenLabel, sizeof(hiddenLabel), "%s", idLabel);
    } else {
        snprintf(hiddenLabel, sizeof(hiddenLabel), "##IconBtn_%s", idLabel ? idLabel : "default");
    }
    return ButtonEx(hiddenLabel, opt);
}

} // namespace parallax
