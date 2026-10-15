#include "parallax/layout/navbar.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace parallax::layout {

static NavSide s_currentNavSide{NavSide::Left};
static ImVec2 s_navPos{0.0f, 0.0f};
static ImVec2 s_navSize{0.0f, 0.0f};
static bool s_navExpanded{false}; // Default collapsed to ~68px icon-only
static float s_animatedNavW{68.0f};

bool NavBar::IsExpanded() {
    return s_navExpanded;
}

void NavBar::SetExpanded(bool expanded) {
    s_navExpanded = expanded;
}

void NavBar::ToggleExpanded() {
    s_navExpanded = !s_navExpanded;
}

float NavBar::GetNavWidth() {
    return s_animatedNavW;
}

bool NavBar::Begin(NavSide side) {
    s_currentNavSide = side;
    Context& ctx = Context::Get();
    ctx.CurrentNavSide = side;

    const float dpi = ctx.GetDpiScale();
    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 winSize = ImGui::GetWindowSize();
    const auto& metrics = StyleManager::Get().Metrics();
    const auto& colors = StyleManager::Get().Colors();

    float navW = 0.0f;
    float navH = 0.0f;

    const bool isVertical = (side == NavSide::Left || side == NavSide::Right);

    if (isVertical) {
        // Smoothly animate width between 68px (collapsed icon-only) and 200px (expanded)
        const float targetW = s_navExpanded ? 200.0f : 68.0f;
        s_animatedNavW = ctx.Anim().SpringFloat(ImGui::GetID("##NavBarWidthSpring"), targetW, {220.0f, 22.0f, 1.0f});
        navW = s_animatedNavW * dpi;
        navH = winSize.y;

        if (side == NavSide::Left) {
            s_navPos = winPos;
        } else {
            s_navPos = ImVec2(winPos.x + winSize.x - navW, winPos.y);
        }
    } else {
        s_animatedNavW = winSize.x / dpi;
        navW = winSize.x;
        navH = metrics.NavHeight * dpi;
        if (side == NavSide::Top) {
            s_navPos = winPos;
        } else {
            s_navPos = ImVec2(winPos.x, winPos.y + winSize.y - navH);
        }
    }

    s_navSize = ImVec2(navW, navH);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float rounding = metrics.WindowRounding * dpi;

    // Fill navigation sidebar background (#11131B Layer)
    drawList->AddRectFilled(
        s_navPos,
        ImVec2(s_navPos.x + s_navSize.x, s_navPos.y + s_navSize.y),
        ImColor(colors.HeaderBg),
        rounding,
        (side == NavSide::Left)   ? (ImDrawFlags_RoundCornersLeft) :
        (side == NavSide::Right)  ? (ImDrawFlags_RoundCornersRight) :
        (side == NavSide::Top)    ? (ImDrawFlags_RoundCornersTop) :
                                    (ImDrawFlags_RoundCornersBottom)
    );

    // Dividing border line
    if (side == NavSide::Left) {
        drawList->AddLine(
            ImVec2(s_navPos.x + s_navSize.x, s_navPos.y),
            ImVec2(s_navPos.x + s_navSize.x, s_navPos.y + s_navSize.y),
            ImColor(colors.Border),
            1.0f
        );
    } else if (side == NavSide::Right) {
        drawList->AddLine(
            ImVec2(s_navPos.x, s_navPos.y),
            ImVec2(s_navPos.x, s_navPos.y + s_navSize.y),
            ImColor(colors.Border),
            1.0f
        );
    } else if (side == NavSide::Top) {
        drawList->AddLine(
            ImVec2(s_navPos.x, s_navPos.y + s_navSize.y),
            ImVec2(s_navPos.x + s_navSize.x, s_navPos.y + s_navSize.y),
            ImColor(colors.Border),
            1.0f
        );
    } else if (side == NavSide::Bottom) {
        drawList->AddLine(
            ImVec2(s_navPos.x, s_navPos.y),
            ImVec2(s_navPos.x + s_navSize.x, s_navPos.y),
            ImColor(colors.Border),
            1.0f
        );
    }

    const float expandT = std::clamp((s_animatedNavW - 68.0f) / (200.0f - 68.0f), 0.0f, 1.0f);

    // Custom Parallax Brand Logo & Header
    if (isVertical) {
        const float logoDim = 28.0f * dpi;
        const float logoX = s_navPos.x + (68.0f * dpi - logoDim) * 0.5f;
        const float logoY = s_navPos.y + 14.0f * dpi;
        const ImVec2 logoPos(logoX, logoY);

        // Render custom Futuristic Layered Neon P Logo with preserveColors = true
        auto logoSub = ctx.Icons().GetIcon("logo", dpi, true);
        if (!logoSub.valid || !logoSub.srv) {
            logoSub = ctx.Icons().GetIcon("layers", dpi, false);
        }
        if (logoSub.valid && logoSub.srv) {
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(logoSub.srv),
                logoPos,
                ImVec2(logoPos.x + logoDim, logoPos.y + logoDim),
                logoSub.uv0, logoSub.uv1,
                IM_COL32(255, 255, 255, 255)
            );
        }

        // Fading brand title and version
        if (expandT > 0.05f) {
            ImVec4 titleCol = colors.TextPrimary;
            titleCol.w = expandT;
            ImVec4 verCol = colors.TextMuted;
            verCol.w = expandT;

            const float textStartX = logoPos.x + logoDim + 10.0f * dpi;
            PushFont(FontWeight::Bold);
            drawList->AddText(ImVec2(textStartX, logoPos.y + 1.0f * dpi), ImColor(titleCol), "PARALLAX");
            PopFont();

            PushFont(FontWeight::Caption);
            drawList->AddText(ImVec2(textStartX, logoPos.y + 16.0f * dpi), ImColor(verCol), "v3.2 SYSTEM");
            PopFont();
        }

        // Animated Chevron Expansion Toggle Button
        const float btnDim = 22.0f * dpi;
        const float btnX = s_navPos.x + (s_navSize.x - btnDim) * 0.5f;
        const float btnY = s_navPos.y + 48.0f * dpi;
        const ImVec2 btnPos(btnX, btnY);
        const ImRect toggleBb(btnPos, ImVec2(btnPos.x + btnDim, btnPos.y + btnDim));

        ImGuiID toggleId = ImGui::GetID("##NavToggleExpansionBtn");
        ImGui::ItemSize(toggleBb);
        if (ImGui::ItemAdd(toggleBb, toggleId)) {
            bool tHovered = false, tHeld = false;
            if (ImGui::ButtonBehavior(toggleBb, toggleId, &tHovered, &tHeld)) {
                s_navExpanded = !s_navExpanded;
            }

            if (tHovered) {
                drawList->AddRectFilled(toggleBb.Min, toggleBb.Max, ImColor(colors.CardHover), 4.0f * dpi);
            }
            drawList->AddRect(toggleBb.Min, toggleBb.Max, ImColor(tHovered ? colors.BorderStrong : colors.Border), 4.0f * dpi, 0, 1.0f);

            const char* chevIcon = s_navExpanded ? "chevron-left" : "chevron-right";
            auto chev = ctx.Icons().GetIcon(chevIcon);
            if (chev.valid && chev.srv) {
                const float cDim = 12.0f * dpi;
                const ImVec2 cPos(btnPos.x + (btnDim - cDim) * 0.5f, btnPos.y + (btnDim - cDim) * 0.5f);
                drawList->AddImage(
                    reinterpret_cast<ImTextureID>(chev.srv),
                    cPos,
                    ImVec2(cPos.x + cDim, cPos.y + cDim),
                    chev.uv0, chev.uv1,
                    ImColor(tHovered ? colors.Accent : colors.TextMuted)
                );
            }

            if (tHovered) {
                ImGui::SetTooltip("%s Sidebar", s_navExpanded ? "Collapse" : "Expand");
            }
        }

        // Set cursor for subsequent tabs
        ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 8.0f * dpi, s_navPos.y + 78.0f * dpi));
    } else {
        // Horizontal top / bottom navbar brand
        ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 18.0f * dpi, s_navPos.y + 12.0f * dpi));
        ImGui::BeginGroup();
        auto logoSub = ctx.Icons().GetIcon("logo", dpi, true);
        if (!logoSub.valid || !logoSub.srv) {
            logoSub = ctx.Icons().GetIcon("layers", dpi, false);
        }
        if (logoSub.valid && logoSub.srv) {
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(logoSub.srv),
                ImVec2(s_navPos.x + 18.0f * dpi, s_navPos.y + 12.0f * dpi),
                ImVec2(s_navPos.x + 18.0f * dpi + 24.0f * dpi, s_navPos.y + 12.0f * dpi + 24.0f * dpi),
                logoSub.uv0, logoSub.uv1,
                IM_COL32(255, 255, 255, 255)
            );
        }
        ImGui::SameLine(0.0f, 10.0f * dpi);
        PushFont(FontWeight::Bold);
        ImGui::TextColored(colors.TextPrimary, "PARALLAX");
        PopFont();
        ImGui::EndGroup();
        ImGui::SameLine(0.0f, 24.0f * dpi);
    }

    return true;
}

void NavBar::End() {
    // End of navbar region
}

bool NavBar::TabItem(const char* icon, const char* label, bool selected, const char* badge) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);
    const bool isVertical = (s_currentNavSide == NavSide::Left || s_currentNavSide == NavSide::Right);

    ImVec2 size;
    if (isVertical) {
        size = ImVec2(s_navSize.x - 16.0f * dpi, 40.0f * dpi);
    } else {
        PushFont(FontWeight::Medium);
        ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
        PopFont();
        size = ImVec2(labelSize.x + 50.0f * dpi + (badge ? 24.0f * dpi : 0.0f), 36.0f * dpi);
    }

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Spring animated hover/select highlight
    const float targetHighlight = selected ? 1.0f : (hovered ? 0.35f : 0.0f);
    const float highlight = ctx.Anim().SpringFloat(id, targetHighlight, {160.0f, 20.0f, 1.0f, 0.001f});

    const float frameRounding = metrics.FrameRounding * dpi;

    // Render active/hover background pill
    if (highlight > 0.01f) {
        ImVec4 pillColor = selected ? colors.AccentMuted : colors.CardHover;
        pillColor.w *= highlight;
        drawList->AddRectFilled(bb.Min, bb.Max, ImColor(pillColor), frameRounding);

        if (selected) {
            if (isVertical) {
                float barX = (s_currentNavSide == NavSide::Left) ? (bb.Min.x + 1.0f * dpi) : (bb.Max.x - 3.0f * dpi);
                drawList->AddRectFilled(
                    ImVec2(barX, bb.Min.y + 8.0f * dpi),
                    ImVec2(barX + 3.0f * dpi, bb.Max.y - 8.0f * dpi),
                    ImColor(colors.Accent),
                    2.0f
                );
            } else {
                drawList->AddRectFilled(
                    ImVec2(bb.Min.x + 10.0f * dpi, bb.Max.y - 3.0f * dpi),
                    ImVec2(bb.Max.x - 10.0f * dpi, bb.Max.y),
                    ImColor(colors.Accent),
                    2.0f
                );
            }
        }
    }

    const ImVec4 iconTint = selected ? colors.Accent : (hovered ? colors.TextPrimary : colors.TextSecondary);
    const ImVec4 textCol = selected ? colors.TextPrimary : (hovered ? colors.TextPrimary : colors.TextSecondary);

    auto sub = ctx.Icons().GetIcon(icon ? icon : "circle");

    if (isVertical) {
        const float expandT = std::clamp((s_animatedNavW - 68.0f) / (200.0f - 68.0f), 0.0f, 1.0f);
        const float iconDim = 18.0f * dpi;

        // Centered icon when collapsed, glides to left as it expands
        const float startIconX = bb.Min.x + (size.x - iconDim) * 0.5f;
        const float targetIconX = bb.Min.x + 14.0f * dpi;
        const float curIconX = startIconX + (targetIconX - startIconX) * expandT;
        const ImVec2 iconPos(curIconX, bb.Min.y + (size.y - iconDim) * 0.5f);

        if (sub.valid && sub.srv) {
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(sub.srv),
                iconPos,
                ImVec2(iconPos.x + iconDim, iconPos.y + iconDim),
                sub.uv0, sub.uv1,
                ImColor(iconTint)
            );
        }

        // Text & Badge fade in with expandT
        if (expandT > 0.05f) {
            PushFont(FontWeight::Medium);
            const float textLineH = ImGui::GetTextLineHeight();
            const ImVec2 textPos(curIconX + 26.0f * dpi, bb.Min.y + (size.y - textLineH) * 0.5f);

            ImVec4 fadeTextCol = textCol;
            fadeTextCol.w *= expandT;
            drawList->AddText(textPos, ImColor(fadeTextCol), label);
            PopFont();

            if (badge && badge[0] != '\0') {
                PushFont(FontWeight::Caption);
                const ImVec2 badgeSize = ImGui::CalcTextSize(badge);
                const ImVec2 badgePos(bb.Max.x - badgeSize.x - 12.0f * dpi, bb.Min.y + (size.y - badgeSize.y) * 0.5f);

                ImVec4 bgCol = colors.AccentMuted;
                bgCol.w *= expandT;
                ImVec4 badgeCol = colors.Accent;
                badgeCol.w *= expandT;

                drawList->AddRectFilled(
                    ImVec2(badgePos.x - 5.0f * dpi, badgePos.y - 2.0f * dpi),
                    ImVec2(badgePos.x + badgeSize.x + 5.0f * dpi, badgePos.y + badgeSize.y + 2.0f * dpi),
                    ImColor(bgCol),
                    4.0f
                );
                drawList->AddText(badgePos, ImColor(badgeCol), badge);
                PopFont();
            }
        }

        // Show hover tooltip when collapsed
        if (expandT < 0.2f && hovered) {
            ImGui::SetTooltip("%s", label);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 4.0f * dpi));
    } else {
        // Horizontal top/bottom tab
        const float iconDim = 16.0f * dpi;
        const ImVec2 iconPos(bb.Min.x + 10.0f * dpi, bb.Min.y + (size.y - iconDim) * 0.5f);
        if (sub.valid && sub.srv) {
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(sub.srv),
                iconPos,
                ImVec2(iconPos.x + iconDim, iconPos.y + iconDim),
                sub.uv0, sub.uv1,
                ImColor(iconTint)
            );
        }

        PushFont(FontWeight::Medium);
        const float textLineH = ImGui::GetTextLineHeight();
        const ImVec2 textPos(iconPos.x + 22.0f * dpi, bb.Min.y + (size.y - textLineH) * 0.5f);
        drawList->AddText(textPos, ImColor(textCol), label);
        PopFont();

        ImGui::SameLine(0.0f, 6.0f * dpi);
    }

    return pressed;
}

} // namespace parallax::layout
