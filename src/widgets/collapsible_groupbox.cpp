#include "parallax/widgets/collapsible_groupbox.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/layout/grid.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace parallax {

static bool s_collapsibleOpenState = false;
static float s_collapsibleProgress = 1.0f;

bool BeginCollapsibleGroupbox(
    const char* title,
    bool* p_expanded,
    const char* icon,
    float defaultHeight,
    bool centered
) {
    if (!p_expanded) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(title ? title : "##CollapsibleGB");

    float cardW = layout::Grid::GetColumnWidth();
    if (cardW <= 0.0f) {
        cardW = ImGui::GetContentRegionAvail().x;
    }

    const float headerH = 34.0f * dpi;
    const float contentH = (defaultHeight > 0.0f) ? (defaultHeight * dpi) : (160.0f * dpi);

    // Spring animated progress: 0.0 (collapsed) to 1.0 (expanded)
    const float targetProgress = *p_expanded ? 1.0f : 0.0f;
    const float progress = ctx.Anim().SpringFloat(id, targetProgress, {160.0f, 20.0f, 1.0f});
    s_collapsibleProgress = progress;

    const float totalH = headerH + (contentH * progress);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect cardBb(pos, ImVec2(pos.x + cardW, pos.y + totalH));

    ImGui::ItemSize(cardBb);
    if (!ImGui::ItemAdd(cardBb, id)) return false;

    // Render outer card background & border
    drawList->AddRectFilled(cardBb.Min, cardBb.Max, ImColor(colors.CardBg), metrics.ChildRounding);
    drawList->AddRect(cardBb.Min, cardBb.Max, ImColor(colors.Border), metrics.ChildRounding, 0, 1.0f);

    // Header interaction button
    const ImRect headerBb(pos, ImVec2(pos.x + cardW, pos.y + headerH));
    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(headerBb, id + 1, &hovered, &held);

    if (pressed) {
        *p_expanded = !(*p_expanded);
    }

    if (hovered) {
        drawList->AddRectFilled(
            headerBb.Min, headerBb.Max,
            ImColor(colors.CardHover),
            metrics.ChildRounding,
            ImDrawFlags_RoundCornersTop
        );
    }

    // Header Content
    const ImVec2 titleSize = ImGui::CalcTextSize(title ? title : "");
    const float midHeaderY = headerBb.Min.y + (headerH - titleSize.y) * 0.5f;

    if (centered) {
        // Centered Title with subtle horizontal divider lines
        const float titleX = headerBb.Min.x + (cardW - titleSize.x) * 0.5f;
        drawList->AddText(ImVec2(titleX, midHeaderY), ImColor(colors.TextPrimary), title);

        // Divider lines on either side
        const float lineY = headerBb.Min.y + headerH * 0.5f;
        const float margin = 14.0f * dpi;
        if (titleX - margin > headerBb.Min.x + 16.0f * dpi) {
            drawList->AddLine(
                ImVec2(headerBb.Min.x + 16.0f * dpi, lineY),
                ImVec2(titleX - margin, lineY),
                ImColor(colors.Border)
            );
        }
        if (titleX + titleSize.x + margin < headerBb.Max.x - 36.0f * dpi) {
            drawList->AddLine(
                ImVec2(titleX + titleSize.x + margin, lineY),
                ImVec2(headerBb.Max.x - 36.0f * dpi, lineY),
                ImColor(colors.Border)
            );
        }
    } else {
        // Left-aligned header with optional icon
        float textX = headerBb.Min.x + 12.0f * dpi;
        if (icon && icon[0] != '\0') {
            auto sub = ctx.Icons().GetIcon(icon);
            if (sub.valid && sub.srv) {
                const float iconDim = 16.0f * dpi;
                const ImVec2 iconPos = ImVec2(textX, headerBb.Min.y + (headerH - iconDim) * 0.5f);
                drawList->AddImage(
                    reinterpret_cast<ImTextureID>(sub.srv),
                    iconPos,
                    ImVec2(iconPos.x + iconDim, iconPos.y + iconDim),
                    sub.uv0, sub.uv1,
                    ImColor(colors.Accent)
                );
                textX += iconDim + 8.0f * dpi;
            }
        }
        drawList->AddText(ImVec2(textX, midHeaderY), ImColor(colors.TextPrimary), title);
    }

    // Chevron icon on right edge indicating expand/collapse state
    const char* chevronIcon = (*p_expanded) ? "chevron-down" : "chevron-right";
    auto chevSub = ctx.Icons().GetIcon(chevronIcon);
    if (chevSub.valid && chevSub.srv) {
        const float chevDim = 14.0f * dpi;
        const ImVec2 chevPos = ImVec2(headerBb.Max.x - chevDim - 12.0f * dpi, headerBb.Min.y + (headerH - chevDim) * 0.5f);
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(chevSub.srv),
            chevPos,
            ImVec2(chevPos.x + chevDim, chevPos.y + chevDim),
            chevSub.uv0, chevSub.uv1,
            ImColor(hovered ? colors.TextPrimary : colors.TextMuted)
        );
    }

    // Divider line below header if expanded
    if (progress > 0.05f) {
        drawList->AddLine(
            ImVec2(headerBb.Min.x, headerBb.Max.y),
            ImVec2(headerBb.Max.x, headerBb.Max.y),
            ImColor(colors.Border)
        );
    }

    s_collapsibleOpenState = (progress > 0.05f);

    if (s_collapsibleOpenState) {
        // Position cursor inside content child region
        ImGui::SetCursorScreenPos(ImVec2(headerBb.Min.x + 12.0f * dpi, headerBb.Max.y + 8.0f * dpi));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        char childId[64];
        snprintf(childId, sizeof(childId), "##CollapsibleContent_%08X", id);

        const float childW = cardW - 24.0f * dpi;
        const float childH = (contentH - 16.0f * dpi) * progress;

        ImGui::BeginChild(childId, ImVec2(childW, childH), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushClipRect(ImVec2(headerBb.Min.x, headerBb.Max.y), cardBb.Max, true);
        return true;
    }

    return false;
}

void EndCollapsibleGroupbox() {
    if (s_collapsibleOpenState) {
        ImGui::PopClipRect();
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}

} // namespace parallax
