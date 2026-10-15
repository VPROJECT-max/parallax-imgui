#include "parallax/layout/tabs.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace parallax::layout {

static SubTabPlacement s_placement{SubTabPlacement::TopHorizontal};
static float s_targetIndicatorX{0.0f};
static float s_targetIndicatorW{0.0f};
static float s_indicatorY{0.0f};
static bool s_hasActiveTabThisFrame{false};

bool SubTabs::Begin(SubTabPlacement placement) {
    s_placement = placement;
    Context& ctx = Context::Get();
    ctx.CurrentSubTabPlacement = placement;

    const float dpi = ctx.GetDpiScale();

    s_hasActiveTabThisFrame = false;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(16.0f * dpi, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f * dpi, 8.0f * dpi));

    return true;
}

bool SubTabs::Item(const char* label, bool selected) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);
    const ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
    const ImVec2 size = ImVec2(labelSize.x + 20.0f * dpi, 32.0f * dpi);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    if (selected) {
        s_targetIndicatorX = bb.Min.x;
        s_targetIndicatorW = bb.GetWidth();
        s_indicatorY = bb.Max.y;
        s_hasActiveTabThisFrame = true;
    }

    // Text color
    ImVec4 textCol = selected ? colors.TextPrimary : (hovered ? colors.TextPrimary : colors.TextSecondary);
    ImVec2 textPos = ImVec2(bb.Min.x + 10.0f * dpi, bb.Min.y + (size.y - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(textPos, ImColor(textCol), label);

    if (s_placement == SubTabPlacement::TopHorizontal || s_placement == SubTabPlacement::BottomHorizontal) {
        ImGui::SameLine(0.0f, 8.0f * dpi);
    } else {
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 4.0f * dpi));
    }

    return pressed;
}

void SubTabs::End() {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (s_placement == SubTabPlacement::TopHorizontal || s_placement == SubTabPlacement::BottomHorizontal) {
        ImGui::NewLine();

        // Base horizontal divider line
        ImVec2 contentMin = ImGui::GetCursorScreenPos();
        float contentW = ImGui::GetContentRegionAvail().x;
        drawList->AddLine(
            ImVec2(contentMin.x, s_indicatorY),
            ImVec2(contentMin.x + contentW, s_indicatorY),
            ImColor(colors.Border),
            1.0f
        );

        // Smoothly sliding and stretching active indicator line using harmonic spring physics
        if (s_hasActiveTabThisFrame) {
            // Differing stiffness/damping between position and width creates natural, organic stretching
            const anim::SpringConfig posCfg{180.0f, 22.0f, 1.0f, 0.001f};
            const anim::SpringConfig widthCfg{140.0f, 18.0f, 1.0f, 0.001f};

            const float curX = ctx.Anim().SpringFloat(ImGui::GetID("##SubTabStretchingIndicatorX"), s_targetIndicatorX, posCfg);
            const float curW = ctx.Anim().SpringFloat(ImGui::GetID("##SubTabStretchingIndicatorW"), s_targetIndicatorW, widthCfg);

            // Draw glowing active indicator bar
            drawList->AddRectFilled(
                ImVec2(curX, s_indicatorY - 1.5f * dpi),
                ImVec2(curX + curW, s_indicatorY + 1.5f * dpi),
                ImColor(colors.Accent),
                2.0f
            );
        }
    }

    ImGui::PopStyleVar(2);
}

} // namespace parallax::layout
