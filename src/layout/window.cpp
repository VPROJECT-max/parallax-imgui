#include "parallax/layout/window.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include "parallax/input/hotkeys.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace parallax::layout {

static ImVec2 s_windowPos{0.0f, 0.0f};
static ImVec2 s_windowSize{0.0f, 0.0f};
static float s_appliedScale{1.0f};
static bool s_closingDisabledPushed{false};

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

static void ScaleWindowDrawListRecursive(ImGuiWindow* win, ImVec2 center, float scale) {
    if (!win) return;
    if (win->DrawList) {
        for (int i = 0; i < win->DrawList->VtxBuffer.Size; ++i) {
            ImDrawVert& v = win->DrawList->VtxBuffer[i];
            v.pos.x = center.x + (v.pos.x - center.x) * scale;
            v.pos.y = center.y + (v.pos.y - center.y) * scale;
        }
    }
    for (int i = 0; i < win->DC.ChildWindows.Size; ++i) {
        ScaleWindowDrawListRecursive(win->DC.ChildWindows[i], center, scale);
    }
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

    // Advance smooth theme & accent transitions
    StyleManager::Get().UpdateTransition(ImGui::GetIO().DeltaTime);

    // Process hotkeys (Right Shift / Insert)
    ctx.Input().Update();

    bool requestOpen = ctx.Input().IsMenuOpen();

    // If menu is closing, dismiss open popups cleanly
    if (!requestOpen) {
        ImGui::ClosePopupsOverWindow(nullptr, false);
    }

    float alpha = 1.0f;
    float scale = 1.0f;
    bool shouldRender = true;

    ctx.Anim().UpdateOpenCloseTransition(requestOpen, alpha, scale, shouldRender);
    ctx.MenuVisibilityAlpha = alpha;
    ctx.MenuScale = scale;
    s_appliedScale = scale;

    // The user's *p_open flag only flips to false when the close animation has fully finished
    if (p_open) {
        *p_open = shouldRender;
        s_lastKnownCallerState = shouldRender;
    }

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

    const auto& colors = StyleManager::Get().Colors();
    const float rounding = StyleManager::Get().Metrics().WindowRounding * dpi;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, colors.ContentBg);

    bool opened = ImGui::Begin(title, nullptr, flags);

    s_windowPos = ImGui::GetWindowPos();
    s_windowSize = ImGui::GetWindowSize();

    // Soft multi-layer ambient drop shadow behind the menu shell
    if (ImDrawList* bgDraw = ImGui::GetBackgroundDrawList()) {
        for (int s = 1; s <= 3; ++s) {
            const float spread = static_cast<float>(s) * 4.0f * dpi;
            ImVec4 shCol = colors.Shadow;
            shCol.w *= alpha * (0.38f / static_cast<float>(s));
            bgDraw->AddRectFilled(
                ImVec2(s_windowPos.x - spread * 0.5f, s_windowPos.y - spread * 0.25f + 2.0f * dpi),
                ImVec2(s_windowPos.x + s_windowSize.x + spread * 0.5f, s_windowPos.y + s_windowSize.y + spread * 0.75f + 2.0f * dpi),
                ImColor(shCol),
                rounding + spread * 0.5f
            );
        }
    }

    // Disable input interactions during closing transition
    s_closingDisabledPushed = !requestOpen;
    if (s_closingDisabledPushed) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }

    return opened;
}

void WindowShell::End() {
    if (s_closingDisabledPushed) {
        ImGui::PopItemFlag();
        s_closingDisabledPushed = false;
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
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

    // Apply cohesive scale transform across the root window AND all child windows
    if (s_appliedScale < 0.999f || s_appliedScale > 1.001f) {
        const ImVec2 center = ImVec2(
            s_windowPos.x + s_windowSize.x * 0.5f,
            s_windowPos.y + s_windowSize.y * 0.5f
        );

        ScaleWindowDrawListRecursive(window, center, s_appliedScale);
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
