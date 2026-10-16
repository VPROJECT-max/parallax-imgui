#include "parallax/layout/tabs.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
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

    PushFont(FontWeight::Medium);
    const ImGuiID id = window->GetID(label);
    const ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
    const ImVec2 size = ImVec2(labelSize.x + 20.0f * dpi, 32.0f * dpi);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) {
        PopFont();
        return false;
    }

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    const float hoverAnim = ctx.Anim().SpringFloat(id ^ 0x55AA11u, (selected || hovered) ? 1.0f : 0.0f, {200.0f, 22.0f, 1.0f});
    if (hoverAnim > 0.01f) {
        ImVec4 pillBg = selected ? colors.AccentMuted : colors.WidgetHover;
        pillBg.w *= hoverAnim;
        drawList->AddRectFilled(
            ImVec2(bb.Min.x, bb.Min.y + 2.0f * dpi),
            ImVec2(bb.Max.x, bb.Max.y - 3.0f * dpi),
            ImColor(pillBg),
            5.0f * dpi
        );
    }

    if (selected) {
        s_targetIndicatorX = bb.Min.x;
        s_targetIndicatorW = bb.GetWidth();
        s_indicatorY = bb.Max.y;
        s_hasActiveTabThisFrame = true;

        if (s_placement == SubTabPlacement::NestedSidebar) {
            const float barX = bb.Min.x;
            drawList->AddRectFilled(
                ImVec2(barX, bb.Min.y + 5.0f * dpi),
                ImVec2(barX + 2.5f * dpi, bb.Max.y - 5.0f * dpi),
                ImColor(colors.Accent),
                1.5f * dpi
            );
        }
    }

    // Text color
    ImVec4 textCol = selected ? colors.TextPrimary : (hovered ? colors.TextPrimary : colors.TextSecondary);
    ImVec2 textPos = ImVec2(bb.Min.x + 10.0f * dpi, bb.Min.y + (size.y - ImGui::GetFontSize()) * 0.5f);
    const char* textEnd = ImGui::FindRenderedTextEnd(label);
    drawList->AddText(nullptr, 0.0f, textPos, ImColor(textCol), label, textEnd);
    PopFont();

    if (s_placement == SubTabPlacement::TopHorizontal || s_placement == SubTabPlacement::BottomHorizontal) {
        ImGui::SameLine(0.0f, 6.0f * dpi);
    } else {
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 4.0f * dpi));
        window->DC.IsSetPos = false;
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
            const anim::SpringConfig posCfg{180.0f, 22.0f, 1.0f, 0.001f};
            const anim::SpringConfig widthCfg{140.0f, 18.0f, 1.0f, 0.001f};

            const float curX = ctx.Anim().SpringFloat(ImGui::GetID("##SubTabStretchingIndicatorX"), s_targetIndicatorX, posCfg);
            const float curW = ctx.Anim().SpringFloat(ImGui::GetID("##SubTabStretchingIndicatorW"), s_targetIndicatorW, widthCfg);

            drawList->AddRectFilled(
                ImVec2(curX + 4.0f * dpi, s_indicatorY - 2.0f * dpi),
                ImVec2(curX + curW - 4.0f * dpi, s_indicatorY + 0.5f * dpi),
                ImColor(colors.Accent),
                2.0f
            );
        }
        ImGui::Dummy(ImVec2(0.0f, 6.0f * dpi));
    }

    ImGui::PopStyleVar(2);
}

} // namespace parallax::layout
