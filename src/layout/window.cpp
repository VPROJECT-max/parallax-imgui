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

constexpr float PARALLAX_FIXED_WIDTH = 1137.0f;
constexpr float PARALLAX_FIXED_HEIGHT = 699.0f;

void WindowShell::SetDefaultSize(float width, float height) {
    (void)width;
    (void)height;
    // Dimensions strictly locked to 1137x699
    Context::Get().DefaultWindowSize = ImVec2(PARALLAX_FIXED_WIDTH, PARALLAX_FIXED_HEIGHT);
}

ImVec2 WindowShell::GetDefaultSize() noexcept {
    return ImVec2(PARALLAX_FIXED_WIDTH, PARALLAX_FIXED_HEIGHT);
}

void WindowShell::SetSizeBounds(float minWidth, float minHeight, float maxWidth, float maxHeight) {
    (void)minWidth; (void)minHeight; (void)maxWidth; (void)maxHeight;
    // Dimensions strictly locked to 1137x699
    Context::Get().MinWindowSize = ImVec2(PARALLAX_FIXED_WIDTH, PARALLAX_FIXED_HEIGHT);
    Context::Get().MaxWindowSize = ImVec2(PARALLAX_FIXED_WIDTH, PARALLAX_FIXED_HEIGHT);
}

ImVec2 WindowShell::GetMinBounds() noexcept {
    return ImVec2(PARALLAX_FIXED_WIDTH, PARALLAX_FIXED_HEIGHT);
}

ImVec2 WindowShell::GetMaxBounds() noexcept {
    return ImVec2(PARALLAX_FIXED_WIDTH, PARALLAX_FIXED_HEIGHT);
}

bool WindowShell::Begin(const char* title, bool* p_open, ImGuiWindowFlags extraFlags) {
    Context& ctx = Context::Get();

    static bool s_lastKnownCallerState = true;
    static bool s_hasInitializedCallerState = false;

    if (p_open) {
        if (!s_hasInitializedCallerState) {
            s_lastKnownCallerState = *p_open;
            ctx.Input().SetMenuOpen(*p_open);
            s_hasInitializedCallerState = true;
        } else if (*p_open != s_lastKnownCallerState) {
            // The caller explicitly changed *p_open programmatically
            ctx.Input().SetMenuOpen(*p_open);
            s_lastKnownCallerState = *p_open;
        }
    }

    // Process hotkeys (Right Shift / Insert)
    ctx.Input().Update();

    bool requestOpen = ctx.Input().IsMenuOpen();
    if (p_open) {
        *p_open = requestOpen;
        s_lastKnownCallerState = requestOpen;
    }

    float alpha = 1.0f;
    float scale = 1.0f;
    bool shouldRender = true;

    ctx.Anim().UpdateOpenCloseTransition(requestOpen, alpha, scale, shouldRender);
    ctx.MenuVisibilityAlpha = alpha;
    ctx.MenuScale = scale;
    s_appliedScale = scale;

    if (!shouldRender) {
        return false;
    }

    const float dpi = ctx.GetDpiScale();
    const ImVec2 fixedSize = ImVec2(PARALLAX_FIXED_WIDTH * dpi, PARALLAX_FIXED_HEIGHT * dpi);

    // Strictly enforce 1137x699 logical pixels at all times
    ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse |
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
