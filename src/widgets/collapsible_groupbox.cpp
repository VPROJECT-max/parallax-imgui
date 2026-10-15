#include "parallax/widgets/collapsible_groupbox.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/layout/grid.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace parallax {

struct CollapsibleFrameState {
    ImGuiID id;
    ImVec2 pos;
    float cardW;
    float headerH;
    float measuredH;
    float progress;
    bool wasDisabledPushed;
    bool isOpen;
};

static std::vector<CollapsibleFrameState> s_stack;
static std::unordered_map<ImGuiID, float> s_measuredHeights;

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

    const float headerH = 36.0f * dpi;

    // Physical spring animated progress: 0.0f (fully collapsed) to 1.0f (fully expanded)
    const float targetProgress = *p_expanded ? 1.0f : 0.0f;
    float progress = ctx.Anim().SpringFloat(id, targetProgress, {260.0f, 26.0f, 1.0f});
    progress = std::clamp(progress, 0.0f, 1.0f);

    // Retrieve dynamically measured content height or default fallback
    auto it = s_measuredHeights.find(id);
    const float measuredH = (it != s_measuredHeights.end()) ? it->second : ((defaultHeight > 0.0f) ? (defaultHeight * dpi) : (150.0f * dpi));
    const float visibleContentH = std::max(measuredH * progress, 0.0f);
    const float totalH = headerH + visibleContentH;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect headerBb(pos, ImVec2(pos.x + cardW, pos.y + headerH));
    const ImRect cardBb(pos, ImVec2(pos.x + cardW, pos.y + totalH));

    // Allocate header item space in parent layout
    ImGui::ItemSize(headerBb);
    if (!ImGui::ItemAdd(headerBb, id)) {
        return false;
    }

    // Header interactive behavior (using same ID registered with ItemAdd)
    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(headerBb, id, &hovered, &held);
    if (pressed) {
        *p_expanded = !(*p_expanded);
    }

    const float rounding = metrics.ChildRounding * dpi;

    // Outer card background and border
    drawList->AddRectFilled(cardBb.Min, cardBb.Max, ImColor(colors.CardBg), rounding);
    drawList->AddRect(cardBb.Min, cardBb.Max, ImColor(colors.Border), rounding, 0, 1.0f);

    // Header hover highlight
    if (hovered) {
        const ImDrawFlags cornerFlags = (progress > 0.05f) ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll;
        drawList->AddRectFilled(headerBb.Min, headerBb.Max, ImColor(colors.CardHover), rounding, cornerFlags);
    }

    // Header Title & Icon
    const ImVec2 titleSize = ImGui::CalcTextSize(title ? title : "");
    const float midHeaderY = headerBb.Min.y + (headerH - titleSize.y) * 0.5f;

    if (centered) {
        const float titleX = headerBb.Min.x + (cardW - titleSize.x) * 0.5f;
        drawList->AddText(ImVec2(titleX, midHeaderY), ImColor(colors.TextPrimary), title);

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

    // Chevron icon indicating expand/collapse state
    const char* chevIcon = (*p_expanded && progress > 0.5f) ? "chevron-down" : "chevron-right";
    auto chevSub = ctx.Icons().GetIcon(chevIcon);
    if (!chevSub.valid || !chevSub.srv) {
        chevSub = ctx.Icons().GetIcon("chevron-down");
    }
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

    // Horizontal divider line below header if expanded
    if (progress > 0.05f) {
        drawList->AddLine(
            ImVec2(headerBb.Min.x, headerBb.Max.y),
            ImVec2(headerBb.Max.x, headerBb.Max.y),
            ImColor(colors.Border)
        );
    }

    // Fully collapsed branch: do not begin child window, return false
    if (!(*p_expanded) && progress <= 0.005f) {
        s_stack.push_back({ id, pos, cardW, headerH, measuredH, 0.0f, false, false });
        return false;
    }

    // Content container branch:
    if (visibleContentH < 1.0f) {
        s_stack.push_back({ id, pos, cardW, headerH, measuredH, progress, false, false });
        return false;
    }

    ImGui::SetCursorScreenPos(ImVec2(headerBb.Min.x, headerBb.Max.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f * dpi, 8.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    char childId[64];
    snprintf(childId, sizeof(childId), "##CGB_Child_%08X", id);
    ImGui::BeginChild(childId, ImVec2(cardW, visibleContentH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);

    // If collapsing, disable child inputs to prevent ghost interactions
    bool pushDisabled = !(*p_expanded);
    if (pushDisabled) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }

    s_stack.push_back({ id, pos, cardW, headerH, measuredH, progress, pushDisabled, true });
    return true;
}

void EndCollapsibleGroupbox() {
    if (s_stack.empty()) return;

    CollapsibleFrameState state = s_stack.back();
    s_stack.pop_back();

    if (!state.isOpen) return;

    if (state.wasDisabledPushed) {
        ImGui::PopItemFlag();
    }

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();

    // Dynamically measure child content height
    ImGuiWindow* childWin = ImGui::GetCurrentWindow();
    if (childWin) {
        const float actualH = (childWin->DC.CursorPos.y - childWin->DC.CursorStartPos.y) + 12.0f * dpi;
        if (actualH > 16.0f * dpi && state.progress > 0.8f) {
            s_measuredHeights[state.id] = actualH;
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg
    ImGui::PopStyleVar(2);  // WindowPadding, ChildRounding
}

} // namespace parallax
