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

bool Button(const char* label, const char* icon, ButtonVariant variant, const ImVec2& sizeArg) {
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

    // Text & Icon sizing
    const ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
    const float iconSize = (icon && icon[0]) ? 16.0f * dpi : 0.0f;
    const float iconSpacing = (icon && icon[0] && labelSize.x > 0.0f) ? 8.0f * dpi : 0.0f;

    const float defaultHeight = 36.0f * dpi;
    const float minWidth = labelSize.x + iconSize + iconSpacing + (metrics.FramePadding.x * 2.0f * dpi);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(sizeArg, minWidth, defaultHeight);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) {
        PopFont();
        return false;
    }

    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Damped harmonic spring physics for hover & active states
    char hoverIdStr[64];
    snprintf(hoverIdStr, sizeof(hoverIdStr), "##btn_hover_%08X", id);
    const float targetHover = hovered ? 1.0f : 0.0f;
    const float animHover = ctx.Anim().SpringFloat(ImGui::GetID(hoverIdStr), targetHover, {120.0f, 18.0f, 1.0f, 0.001f});

    char activeIdStr[64];
    snprintf(activeIdStr, sizeof(activeIdStr), "##btn_active_%08X", id);
    const float targetActive = held ? 1.0f : 0.0f;
    const float animActive = ctx.Anim().SpringFloat(ImGui::GetID(activeIdStr), targetActive, {140.0f, 20.0f, 1.0f, 0.001f});

    ImDrawList* drawList = window->DrawList;
    const float rounding = metrics.FrameRounding * dpi;

    // Determine colors based on variant
    ImVec4 bgCol, borderCol, textCol;

    switch (variant) {
        case ButtonVariant::Primary: {
            bgCol = ImVec4(
                colors.Accent.x * (1.0f + 0.15f * animHover) - 0.1f * animActive,
                colors.Accent.y * (1.0f + 0.15f * animHover) - 0.1f * animActive,
                colors.Accent.z * (1.0f + 0.15f * animHover) - 0.1f * animActive,
                1.0f
            );
            borderCol = ImVec4(colors.AccentHover.x, colors.AccentHover.y, colors.AccentHover.z, 0.4f + 0.4f * animHover);
            textCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }
        case ButtonVariant::Danger: {
            const float rVal = colors.Danger.x;
            const float gVal = colors.Danger.y;
            const float bVal = colors.Danger.z;
            bgCol = ImVec4(rVal * 0.18f + 0.1f * animHover, gVal * 0.05f, bVal * 0.05f, 0.95f);
            borderCol = ImVec4(rVal, gVal, bVal, 0.35f + 0.45f * animHover);
            textCol = ImVec4(1.0f, 0.75f + 0.25f * animHover, 0.75f + 0.25f * animHover, 1.0f);
            break;
        }
        case ButtonVariant::Ghost: {
            bgCol = ImVec4(colors.CardBg.x, colors.CardBg.y, colors.CardBg.z, 0.5f * animHover);
            borderCol = ImVec4(1.0f, 1.0f, 1.0f, 0.05f + 0.15f * animHover);
            textCol = ImVec4(
                colors.TextSecondary.x + (colors.TextPrimary.x - colors.TextSecondary.x) * animHover,
                colors.TextSecondary.y + (colors.TextPrimary.y - colors.TextSecondary.y) * animHover,
                colors.TextSecondary.z + (colors.TextPrimary.z - colors.TextSecondary.z) * animHover,
                1.0f
            );
            break;
        }
        case ButtonVariant::Secondary:
        default: {
            const float lift = 0.04f * animHover - 0.02f * animActive;
            bgCol = ImVec4(
                colors.CardBg.x + lift + 0.02f,
                colors.CardBg.y + lift + 0.02f,
                colors.CardBg.z + lift + 0.03f,
                1.0f
            );
            borderCol = ImVec4(1.0f, 1.0f, 1.0f, 0.08f + 0.12f * animHover);
            textCol = colors.TextPrimary;
            break;
        }
    }

    // Optional subtle outer glow on primary hover
    if (variant == ButtonVariant::Primary && animHover > 0.05f) {
        drawList->AddRect(
            ImVec2(bb.Min.x - 1.0f, bb.Min.y - 1.0f),
            ImVec2(bb.Max.x + 1.0f, bb.Max.y + 1.0f),
            ImColor(colors.Accent.x, colors.Accent.y, colors.Accent.z, 0.2f * animHover),
            rounding + 1.0f,
            0,
            2.0f
        );
    }

    // Render button background & border
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(bgCol), rounding);
    drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), rounding, 0, 1.0f);

    // Compute centered content position
    const float totalContentWidth = iconSize + iconSpacing + labelSize.x;
    float startX = bb.Min.x + (size.x - totalContentWidth) * 0.5f;
    const float centerY = bb.Min.y + (size.y * 0.5f);

    // Render optional icon
    if (icon && icon[0]) {
        const ImVec2 iconPos(startX, centerY - (iconSize * 0.5f));
        ctx.Icons().DrawIcon(icon, ImVec2(iconSize, iconSize), textCol);
        startX += iconSize + iconSpacing;
    }

    // Render label
    if (labelSize.x > 0.0f) {
        const ImVec2 textPos(startX, centerY - (labelSize.y * 0.5f));
        drawList->AddText(textPos, ImColor(textCol), label);
    }

    PopFont();
    return pressed;
}

} // namespace parallax
