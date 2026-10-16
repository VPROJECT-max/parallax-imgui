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

    // Subtle elevation shadow beneath card
    ImVec4 shadowCol = colors.Shadow;
    shadowCol.w *= 0.28f;
    drawList->AddRectFilled(
        ImVec2(cardBb.Min.x, cardBb.Min.y + 2.0f * dpi),
        ImVec2(cardBb.Max.x, cardBb.Max.y + 2.0f * dpi),
        ImColor(shadowCol),
        rounding
    );

    // Outer card background and border
    drawList->AddRectFilled(cardBb.Min, cardBb.Max, ImColor(colors.CardBg), rounding);

    // Header sub-surface (CardHeaderBg when expanded, CardHover when hovered)
    const ImDrawFlags cornerFlags = (progress > 0.05f) ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll;
    if (hovered || progress > 0.05f) {
        const ImVec4 hdrBg = hovered ? colors.CardHover : colors.CardHeaderBg;
        drawList->AddRectFilled(headerBb.Min, headerBb.Max, ImColor(hdrBg), rounding, cornerFlags);
    }

    drawList->AddRect(cardBb.Min, cardBb.Max, ImColor(hovered ? colors.BorderStrong : colors.Border), rounding, 0, 1.0f);

    // Header Title & Icon
    PushFont(FontWeight::SemiBold);
    const ImVec2 titleSize = ImGui::CalcTextSize(title ? title : "", nullptr, true);
    const float midHeaderY = headerBb.Min.y + (headerH - titleSize.y) * 0.5f;

    if (centered) {
        const float titleX = headerBb.Min.x + (cardW - titleSize.x) * 0.5f;
        const char* textEnd = ImGui::FindRenderedTextEnd(title);
        drawList->AddText(nullptr, 0.0f, ImVec2(titleX, midHeaderY), ImColor(colors.TextPrimary), title, textEnd);

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
            const float iconDim = 15.0f * dpi;
            const ImVec2 iconPos = ImVec2(textX, headerBb.Min.y + (headerH - iconDim) * 0.5f);
            ctx.Icons().DrawIcon(drawList, icon, iconPos, ImVec2(iconDim, iconDim), colors.Accent);
            textX += iconDim + 8.0f * dpi;
        }
        const char* textEnd = ImGui::FindRenderedTextEnd(title);
        drawList->AddText(nullptr, 0.0f, ImVec2(textX, midHeaderY), ImColor(colors.TextPrimary), title, textEnd);
    }
    PopFont();

    // Smooth vector chevron indicating expand/collapse progress
    const ImVec2 chevCenter(headerBb.Max.x - 16.0f * dpi, headerBb.Min.y + headerH * 0.5f);
    const float span = 4.0f * dpi;
    const ImU32 chevCol = ImColor(hovered ? colors.Accent : colors.TextSecondary);
    // Interpolate between right-pointing chevron (progress=0) and down-pointing chevron (progress=1)
    const ImVec2 p1(
        chevCenter.x - span * (0.4f + 0.6f * progress),
        chevCenter.y - span * (0.9f - 0.4f * progress)
    );
    const ImVec2 p2(
        chevCenter.x + span * (0.5f * (1.0f - progress)),
        chevCenter.y + span * (0.5f * progress)
    );
    const ImVec2 p3(
        chevCenter.x + span * (-0.4f * (1.0f - progress) + 1.0f * progress),
        chevCenter.y + span * (0.9f * (1.0f - progress) - 0.5f * progress)
    );
    drawList->AddLine(p1, p2, chevCol, 1.6f * dpi);
    drawList->AddLine(p2, p3, chevCol, 1.6f * dpi);

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
