#include "parallax/layout/window.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include "parallax/input/hotkeys.hpp"
#include <algorithm>

namespace parallax::layout {

static ImVec2 s_windowPos{0.0f, 0.0f};
static ImVec2 s_windowSize{0.0f, 0.0f};
static float s_appliedScale{1.0f};
static int s_vtxStartIndex{0};

void WindowShell::SetDefaultSize(float width, float height) {
    Context::Get().DefaultWindowSize = ImVec2(width, height);
}

ImVec2 WindowShell::GetDefaultSize() noexcept {
    return Context::Get().DefaultWindowSize;
}

void WindowShell::SetSizeBounds(float minWidth, float minHeight, float maxWidth, float maxHeight) {
    Context::Get().MinWindowSize = ImVec2(minWidth, minHeight);
    Context::Get().MaxWindowSize = ImVec2(maxWidth, maxHeight);
}

ImVec2 WindowShell::GetMinBounds() noexcept {
    return Context::Get().MinWindowSize;
}

ImVec2 WindowShell::GetMaxBounds() noexcept {
    return Context::Get().MaxWindowSize;
}

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

    const float dpi = ctx.GetDpiScale();
    const ImVec2 defaultSize = ImVec2(ctx.DefaultWindowSize.x * dpi, ctx.DefaultWindowSize.y * dpi);
    const ImVec2 minSize = ImVec2(ctx.MinWindowSize.x * dpi, ctx.MinWindowSize.y * dpi);
    const ImVec2 maxSize = ImVec2(ctx.MaxWindowSize.x * dpi, ctx.MaxWindowSize.y * dpi);

    ImGui::SetNextWindowSize(defaultSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(minSize, maxSize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             extraFlags;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, StyleManager::Get().Metrics().WindowRounding * dpi);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, StyleManager::Get().Colors().Border);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, StyleManager::Get().Colors().WindowBg);

    bool opened = ImGui::Begin(title, nullptr, flags);

    s_windowPos = ImGui::GetWindowPos();
    s_windowSize = ImGui::GetWindowSize();
    s_vtxStartIndex = ImGui::GetWindowDrawList()->VtxBuffer.Size;

    return opened;
}

void WindowShell::End() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float dpi = Context::Get().GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const float rounding = StyleManager::Get().Metrics().WindowRounding * dpi;

    // Subtle 1px structural outer border
    drawList->AddRect(
        s_windowPos,
        ImVec2(s_windowPos.x + s_windowSize.x, s_windowPos.y + s_windowSize.y),
        ImColor(colors.Border),
        rounding,
        0,
        1.0f
    );

    // Modern Corner Drag Resize Grip Indicator (Bottom-Right)
    const float gripAreaSize = 20.0f * dpi;
    const ImVec2 gripCorner(s_windowPos.x + s_windowSize.x, s_windowPos.y + s_windowSize.y);
    const ImVec2 mousePos = ImGui::GetIO().MousePos;

    const bool gripHovered = (mousePos.x >= gripCorner.x - gripAreaSize && mousePos.x <= gripCorner.x &&
                              mousePos.y >= gripCorner.y - gripAreaSize && mousePos.y <= gripCorner.y);

    const float targetGrip = gripHovered ? 1.0f : 0.0f;
    const float animGrip = Context::Get().Anim().SpringFloat(ImGui::GetID("##CornerResizeGrip"), targetGrip, {140.0f, 20.0f, 1.0f});

    const ImU32 gripCol = ImColor(
        colors.BorderStrong.x + (colors.AccentHover.x - colors.BorderStrong.x) * animGrip,
        colors.BorderStrong.y + (colors.AccentHover.y - colors.BorderStrong.y) * animGrip,
        colors.BorderStrong.z + (colors.AccentHover.z - colors.BorderStrong.z) * animGrip,
        0.30f + 0.60f * animGrip
    );

    // Draw three subtle ergonomic diagonal grip lines
    for (int i = 1; i <= 3; ++i) {
        const float offset = (static_cast<float>(i) * 4.0f + 3.0f) * dpi;
        drawList->AddLine(
            ImVec2(gripCorner.x - offset, gripCorner.y - 4.0f * dpi),
            ImVec2(gripCorner.x - 4.0f * dpi, gripCorner.y - offset),
            gripCol,
            1.2f * dpi
        );
    }

    // Apply smooth scale transform to window vertices if in open/close transition
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
    ImGui::PopStyleColor(2); // WindowBg, Border
    ImGui::PopStyleVar(3);   // WindowBorderSize, WindowRounding, Alpha
}

ImVec2 WindowShell::GetContentPosition() noexcept {
    return s_windowPos;
}

ImVec2 WindowShell::GetContentSize() noexcept {
    return s_windowSize;
}

} // namespace parallax::layout
