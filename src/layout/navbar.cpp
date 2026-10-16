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

    // Custom Parallax Brand Emblem & Header Controls
    if (isVertical) {
        const float logoDim = 28.0f * dpi;
        const float logoX = s_navPos.x + (68.0f * dpi - logoDim) * 0.5f;
        const float logoY = s_navPos.y + 13.0f * dpi;
        const ImVec2 logoPos(logoX, logoY);
        const ImVec2 logoMax(logoPos.x + logoDim, logoPos.y + logoDim);

        // Accent-adaptive architectural brand squircle badge
        drawList->AddRectFilled(logoPos, logoMax, ImColor(colors.AccentMuted), 7.0f * dpi);
        drawList->AddRect(logoPos, logoMax, ImColor(colors.Accent), 7.0f * dpi, 0, 1.0f);
        const float innerIconDim = 16.0f * dpi;
        const ImVec2 innerIconPos(logoPos.x + (logoDim - innerIconDim) * 0.5f, logoPos.y + (logoDim - innerIconDim) * 0.5f);
        ctx.Icons().DrawIcon(drawList, "layers", innerIconPos, ImVec2(innerIconDim, innerIconDim), colors.Accent);

        // Fading brand title and SemVer version
        if (expandT > 0.05f) {
            ImVec4 titleCol = colors.TextPrimary;
            titleCol.w = expandT;
            ImVec4 verCol = colors.TextMuted;
            verCol.w = expandT;

            const float textStartX = logoPos.x + logoDim + 10.0f * dpi;
            PushFont(FontWeight::Bold);
            drawList->AddText(ImVec2(textStartX, logoPos.y + 0.5f * dpi), ImColor(titleCol), "PARALLAX");
            PopFont();

            PushFont(FontWeight::Caption);
            drawList->AddText(ImVec2(textStartX, logoPos.y + 15.5f * dpi), ImColor(verCol), "v" PARALLAX_VERSION_STRING " UI");
            PopFont();
        }

        // Utility Row: Sidebar Expand/Collapse + Quick Theme Switcher (Graphite <-> Porcelain)
        const float availRowW = s_navSize.x - 16.0f * dpi;
        const float btnGap = 6.0f * dpi;
        const float eachBtnW = (availRowW - btnGap) * 0.5f;
        const float btnH = 24.0f * dpi;
        const float btnY = s_navPos.y + 47.0f * dpi;

        // 1. Expand / Collapse Button
        {
            const ImVec2 btnPos(s_navPos.x + 8.0f * dpi, btnY);
            const ImRect toggleBb(btnPos, ImVec2(btnPos.x + eachBtnW, btnPos.y + btnH));
            ImGuiID toggleId = ImGui::GetID("##NavToggleExpansionBtn");
            ImGui::ItemSize(toggleBb);
            if (ImGui::ItemAdd(toggleBb, toggleId)) {
                bool tHovered = false, tHeld = false;
                if (ImGui::ButtonBehavior(toggleBb, toggleId, &tHovered, &tHeld)) {
                    s_navExpanded = !s_navExpanded;
                }

                drawList->AddRectFilled(
                    toggleBb.Min, toggleBb.Max,
                    ImColor(tHovered ? colors.WidgetHover : colors.WidgetBg),
                    5.0f * dpi
                );
                drawList->AddRect(
                    toggleBb.Min, toggleBb.Max,
                    ImColor(tHovered ? colors.BorderStrong : colors.Border),
                    5.0f * dpi, 0, 1.0f
                );

                const char* chevIcon = s_navExpanded ? "chevron-left" : "chevron-right";
                const float cDim = 12.0f * dpi;
                const float iconX = (expandT > 0.55f)
                    ? (toggleBb.Min.x + 8.0f * dpi)
                    : (toggleBb.Min.x + (eachBtnW - cDim) * 0.5f);
                const ImVec2 cPos(iconX, toggleBb.Min.y + (btnH - cDim) * 0.5f);
                ctx.Icons().DrawIcon(drawList, chevIcon, cPos, ImVec2(cDim, cDim), tHovered ? colors.Accent : colors.TextSecondary);

                if (expandT > 0.55f) {
                    PushFont(FontWeight::Caption);
                    ImVec4 lblCol = tHovered ? colors.TextPrimary : colors.TextSecondary;
                    lblCol.w *= std::clamp((expandT - 0.55f) / 0.45f, 0.0f, 1.0f);
                    const float th = ImGui::GetFontSize();
                    drawList->AddText(
                        ImVec2(cPos.x + cDim + 5.0f * dpi, toggleBb.Min.y + (btnH - th) * 0.5f),
                        ImColor(lblCol),
                        "Fold"
                    );
                    PopFont();
                }

                if (tHovered) {
                    ImGui::SetTooltip("%s Sidebar", s_navExpanded ? "Collapse" : "Expand");
                }
            }
        }

        // 2. Theme Switcher Button (Dark Graphite <-> Light Porcelain)
        {
            const ImVec2 btnPos(s_navPos.x + 8.0f * dpi + eachBtnW + btnGap, btnY);
            const ImRect themeBb(btnPos, ImVec2(btnPos.x + eachBtnW, btnPos.y + btnH));
            ImGuiID themeId = ImGui::GetID("##NavThemeSwitchBtn");
            ImGui::ItemSize(themeBb);
            if (ImGui::ItemAdd(themeBb, themeId)) {
                bool thHovered = false, thHeld = false;
                if (ImGui::ButtonBehavior(themeBb, themeId, &thHovered, &thHeld)) {
                    StyleManager::Get().ToggleTheme();
                }

                const bool isLight = StyleManager::Get().IsLightTheme();
                drawList->AddRectFilled(
                    themeBb.Min, themeBb.Max,
                    ImColor(thHovered ? colors.WidgetHover : colors.WidgetBg),
                    5.0f * dpi
                );
                drawList->AddRect(
                    themeBb.Min, themeBb.Max,
                    ImColor(thHovered ? colors.Accent : colors.Border),
                    5.0f * dpi, 0, 1.0f
                );

                const char* themeIcon = isLight ? "sun" : "moon";
                const float cDim = 12.0f * dpi;
                const float iconX = (expandT > 0.55f)
                    ? (themeBb.Min.x + 8.0f * dpi)
                    : (themeBb.Min.x + (eachBtnW - cDim) * 0.5f);
                const ImVec2 cPos(iconX, themeBb.Min.y + (btnH - cDim) * 0.5f);
                ctx.Icons().DrawIcon(drawList, themeIcon, cPos, ImVec2(cDim, cDim), thHovered ? colors.Accent : colors.TextSecondary);

                if (expandT > 0.55f) {
                    PushFont(FontWeight::Caption);
                    ImVec4 lblCol = thHovered ? colors.TextPrimary : colors.TextSecondary;
                    lblCol.w *= std::clamp((expandT - 0.55f) / 0.45f, 0.0f, 1.0f);
                    const float th = ImGui::GetFontSize();
                    drawList->AddText(
                        ImVec2(cPos.x + cDim + 5.0f * dpi, themeBb.Min.y + (btnH - th) * 0.5f),
                        ImColor(lblCol),
                        isLight ? "Light" : "Dark"
                    );
                    PopFont();
                }

                if (thHovered) {
                    ImGui::SetTooltip("Switch to %s Theme", isLight ? "Graphite (Dark)" : "Porcelain (Light)");
                }
            }
        }

        // Subtle divider below header utility row
        drawList->AddLine(
            ImVec2(s_navPos.x + 10.0f * dpi, s_navPos.y + 77.0f * dpi),
            ImVec2(s_navPos.x + s_navSize.x - 10.0f * dpi, s_navPos.y + 77.0f * dpi),
            ImColor(colors.BorderSubtle),
            1.0f
        );

        // Set cursor for subsequent tabs
        ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 8.0f * dpi, s_navPos.y + 84.0f * dpi));
    } else {
        // Horizontal top / bottom navbar brand
        ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 18.0f * dpi, s_navPos.y + 12.0f * dpi));
        ImGui::BeginGroup();
        const ImVec2 logoPos(s_navPos.x + 18.0f * dpi, s_navPos.y + 12.0f * dpi);
        const float logoDim = 24.0f * dpi;
        drawList->AddRectFilled(logoPos, ImVec2(logoPos.x + logoDim, logoPos.y + logoDim), ImColor(colors.AccentMuted), 6.0f * dpi);
        drawList->AddRect(logoPos, ImVec2(logoPos.x + logoDim, logoPos.y + logoDim), ImColor(colors.Accent), 6.0f * dpi, 0, 1.0f);
        ctx.Icons().DrawIcon(drawList, "layers", ImVec2(logoPos.x + 4.0f * dpi, logoPos.y + 4.0f * dpi), ImVec2(16.0f * dpi, 16.0f * dpi), colors.Accent);
        ImGui::Dummy(ImVec2(logoDim, logoDim));
        ImGui::SameLine(0.0f, 10.0f * dpi);
        PushFont(FontWeight::Bold);
        ImGui::TextColored(colors.TextPrimary, "PARALLAX");
        PopFont();
        ImGui::EndGroup();
        ImGui::SameLine(0.0f, 20.0f * dpi);
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
        const float collapsedTabW = (68.0f - 16.0f) * dpi;
        const float startIconX = bb.Min.x + (collapsedTabW - iconDim) * 0.5f;
        const float targetIconX = bb.Min.x + 14.0f * dpi;
        const float curIconX = startIconX + (targetIconX - startIconX) * expandT;
        const ImVec2 iconPos(curIconX, bb.Min.y + (size.y - iconDim) * 0.5f);

        ctx.Icons().DrawIcon(drawList, icon ? icon : "circle", iconPos, ImVec2(iconDim, iconDim), iconTint);

        // Text & Badge fade in with expandT
        if (expandT > 0.05f) {
            PushFont(FontWeight::Medium);
            const float textLineH = ImGui::GetTextLineHeight();
            const ImVec2 textPos(targetIconX + iconDim + 10.0f * dpi, bb.Min.y + (size.y - textLineH) * 0.5f);

            ImVec4 fadeTextCol = textCol;
            fadeTextCol.w *= expandT;
            const char* textEnd = ImGui::FindRenderedTextEnd(label);
            drawList->AddText(nullptr, 0.0f, textPos, ImColor(fadeTextCol), label, textEnd);
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
        ctx.Icons().DrawIcon(drawList, icon ? icon : "circle", iconPos, ImVec2(iconDim, iconDim), iconTint);

        PushFont(FontWeight::Medium);
        const float textLineH = ImGui::GetTextLineHeight();
        const ImVec2 textPos(iconPos.x + iconDim + 8.0f * dpi, bb.Min.y + (size.y - textLineH) * 0.5f);
        const char* textEnd = ImGui::FindRenderedTextEnd(label);
        drawList->AddText(nullptr, 0.0f, textPos, ImColor(textCol), label, textEnd);
        PopFont();

        ImGui::SameLine(0.0f, 6.0f * dpi);
    }

    return pressed;
}

} // namespace parallax::layout
