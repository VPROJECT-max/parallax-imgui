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
    ImVec2 cardMin;
    float cardW;
    float headerH;
    float progress;
    bool open;
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

    const float headerH = 34.0f * dpi;

    // Spring animated progress: 0.0 (collapsed) to 1.0 (expanded)
    const float targetProgress = *p_expanded ? 1.0f : 0.0f;
    const float progress = ctx.Anim().SpringFloat(id, targetProgress, {220.0f, 22.0f, 1.0f});

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect headerBb(pos, ImVec2(pos.x + cardW, pos.y + headerH));

    // Fully collapsed branch: only allocate header space, following widgets reclaim layout space
    if (!(*p_expanded) && progress < 0.01f) {
        ImGui::ItemSize(headerBb);
        if (!ImGui::ItemAdd(headerBb, id)) return false;

        // Render fully rounded collapsed card
        drawList->AddRectFilled(headerBb.Min, headerBb.Max, ImColor(colors.CardBg), metrics.ChildRounding);
        drawList->AddRect(headerBb.Min, headerBb.Max, ImColor(colors.Border), metrics.ChildRounding, 0, 1.0f);

        // Header interaction
        bool hovered = false;
        bool held = false;
        bool pressed = ImGui::ButtonBehavior(headerBb, id + 1, &hovered, &held);
        if (pressed) {
            *p_expanded = true;
        }

        if (hovered) {
            drawList->AddRectFilled(
                headerBb.Min, headerBb.Max,
                ImColor(colors.CardHover),
                metrics.ChildRounding
            );
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

        // Chevron icon pointing right (collapsed)
        auto chevSub = ctx.Icons().GetIcon("chevron-right");
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

        return false;
    }

    // Expanded / Animating branch:
    auto it = s_measuredHeights.find(id);
    const float measuredH = (it != s_measuredHeights.end()) ? it->second : ((defaultHeight > 0.0f) ? (defaultHeight * dpi) : (140.0f * dpi));
    const float visibleContentH = std::max(measuredH * progress, 0.0f);
    const float totalH = headerH + visibleContentH;

    const ImRect cardBb(pos, ImVec2(pos.x + cardW, pos.y + totalH));

    // Render outer card background & border behind content
    drawList->AddRectFilled(cardBb.Min, cardBb.Max, ImColor(colors.CardBg), metrics.ChildRounding);
    drawList->AddRect(cardBb.Min, cardBb.Max, ImColor(colors.Border), metrics.ChildRounding, 0, 1.0f);

    // Header item and interaction
    ImGui::ItemSize(headerBb);
    if (!ImGui::ItemAdd(headerBb, id)) return false;

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

    // Chevron icon indicating state
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

    // Begin child clipping window for inner contents
    ImGui::SetCursorScreenPos(ImVec2(headerBb.Min.x, headerBb.Max.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f * dpi, 8.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    char childId[64];
    snprintf(childId, sizeof(childId), "##CGB_Child_%08X", id);
    ImGui::BeginChild(childId, ImVec2(cardW, visibleContentH), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    s_stack.push_back({ id, pos, cardW, headerH, progress, true });
    return true;
}

void EndCollapsibleGroupbox() {
    if (s_stack.empty()) return;

    CollapsibleFrameState state = s_stack.back();
    s_stack.pop_back();

    if (!state.open) return;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();

    // Measure actual rendered child height dynamically
    ImGuiWindow* childWin = ImGui::GetCurrentWindow();
    if (childWin) {
        const float actualH = (childWin->DC.CursorPos.y - childWin->DC.CursorStartPos.y) + 12.0f * dpi;
        if (actualH > 16.0f * dpi) {
            s_measuredHeights[state.id] = actualH;
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg
    ImGui::PopStyleVar(2);  // WindowPadding, ChildRounding
}

} // namespace parallax
