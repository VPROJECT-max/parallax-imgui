#include "parallax/layout/window.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/input/hotkeys.hpp"
#include <algorithm>

namespace parallax::layout {

static ImVec2 s_windowPos{0.0f, 0.0f};
static ImVec2 s_windowSize{0.0f, 0.0f};
static float s_appliedScale{1.0f};
static int s_vtxStartIndex{0};

bool WindowShell::Begin(const char* title, bool* p_open, ImGuiWindowFlags extraFlags) {
    Context& ctx = Context::Get();
    ctx.Input().Update();

    if (p_open) {
        if (*p_open != ctx.Input().IsMenuOpen()) {
            ctx.Input().SetMenuOpen(*p_open);
        }
    }

    bool requestOpen = ctx.Input().IsMenuOpen();
    float alpha = 1.0f;
    float scale = 1.0f;
    bool shouldRender = true;

    ctx.Anim().UpdateOpenCloseTransition(requestOpen, alpha, scale, shouldRender);
    ctx.MenuVisibilityAlpha = alpha;
    ctx.MenuScale = scale;
    s_appliedScale = scale;

    if (!shouldRender) {
        if (p_open) *p_open = false;
        return false;
    }

    if (p_open) *p_open = requestOpen;

    // Smooth tier animated resizing
    const float dpi = ctx.GetDpiScale();
    const ImVec2 rawDims = WindowTierDimensions::GetDimensions(ctx.CurrentSizeTier);
    const ImVec2 targetDimensions = ImVec2(rawDims.x * dpi, rawDims.y * dpi);
    const ImVec2 animatedSize = ctx.Anim().SpringVec2(ImGui::GetID("##ParallaxShellSize"), targetDimensions);

    const ImVec2 minBounds = WindowTierDimensions::MinBounds;
    ImGui::SetNextWindowSize(animatedSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowSizeConstraints(ImVec2(minBounds.x * dpi, minBounds.y * dpi), ImVec2(2560.0f, 1440.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             extraFlags;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, StyleManager::Get().Metrics().WindowRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, StyleManager::Get().Colors().Border);

    bool opened = ImGui::Begin(title, nullptr, flags);

    s_windowPos = ImGui::GetWindowPos();
    s_windowSize = ImGui::GetWindowSize();
    s_vtxStartIndex = ImGui::GetWindowDrawList()->VtxBuffer.Size;

    return opened;
}

void WindowShell::End() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGuiStyle& style = ImGui::GetStyle();

    // Subtle 1px structural outer border
    drawList->AddRect(
        s_windowPos,
        ImVec2(s_windowPos.x + s_windowSize.x, s_windowPos.y + s_windowSize.y),
        ImColor(StyleManager::Get().Colors().Border),
        style.WindowRounding,
        0,
        1.0f
    );

    // Apply smooth scale transform to window vertices if in transition
    if (s_appliedScale < 0.999f || s_appliedScale > 1.001f) {
        const ImVec2 center = ImVec2(
            s_windowPos.x + s_windowSize.x * 0.5f,
            s_windowPos.y + s_windowSize.y * 0.5f
        );

        for (int i = s_vtxStartIndex; i < drawList->VtxBuffer.Size; ++i) {
            ImDrawVert& v = drawList->VtxBuffer[i];
            v.pos.x = center.x + (v.pos.x - center.x) * s_appliedScale;
            v.pos.y = center.y + (v.pos.y - center.y) * s_appliedScale;
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(); // Border
    ImGui::PopStyleVar(3);  // WindowBorderSize, WindowRounding, Alpha
}

ImVec2 WindowShell::GetContentPosition() noexcept {
    return s_windowPos;
}

ImVec2 WindowShell::GetContentSize() noexcept {
    return s_windowSize;
}

} // namespace parallax::layout
