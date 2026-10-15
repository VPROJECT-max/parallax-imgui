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

    switch (side) {
        case NavSide::Left:
            s_navPos = winPos;
            navW = metrics.NavWidth * dpi;
            navH = winSize.y;
            break;
        case NavSide::Right:
            navW = metrics.NavWidth * dpi;
            navH = winSize.y;
            s_navPos = ImVec2(winPos.x + winSize.x - navW, winPos.y);
            break;
        case NavSide::Top:
            s_navPos = winPos;
            navW = winSize.x;
            navH = metrics.NavHeight * dpi;
            break;
        case NavSide::Bottom:
            navW = winSize.x;
            navH = metrics.NavHeight * dpi;
            s_navPos = ImVec2(winPos.x, winPos.y + winSize.y - navH);
            break;
    }

    s_navSize = ImVec2(navW, navH);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const float rounding = metrics.WindowRounding * dpi;

    // Fill navbar background
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

    // Subtle 1px dividing border
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

    // Brand Logo & Title
    if (side == NavSide::Left || side == NavSide::Right) {
        const bool isCompact = (s_navSize.x < 100.0f * dpi);
        if (isCompact) {
            // Centered icon-only brand mark
            const ImVec2 brandPos(s_navPos.x + (s_navSize.x - 22.0f * dpi) * 0.5f, s_navPos.y + 16.0f * dpi);
            ImGui::SetCursorScreenPos(brandPos);
            ctx.Icons().DrawIcon("layers", ImVec2(22.0f * dpi, 22.0f * dpi), colors.Accent);
            ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 8.0f * dpi, s_navPos.y + 54.0f * dpi));
        } else {
            ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 16.0f * dpi, s_navPos.y + 16.0f * dpi));
            ImGui::BeginGroup();
            ctx.Icons().DrawIcon("layers", ImVec2(22.0f * dpi, 22.0f * dpi), colors.Accent);
            ImGui::SameLine(0.0f, 10.0f * dpi);

            PushFont(FontWeight::Bold);
            ImGui::TextColored(colors.TextPrimary, "PARALLAX");
            PopFont();

            ImGui::SameLine(0.0f, 6.0f * dpi);
            PushFont(FontWeight::Caption);
            ImGui::TextColored(colors.TextMuted, "v3.0");
            PopFont();
            ImGui::EndGroup();

            ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 8.0f * dpi, s_navPos.y + 58.0f * dpi));
        }
    } else {
        // Horizontal top / bottom navbar brand
        ImGui::SetCursorScreenPos(ImVec2(s_navPos.x + 18.0f * dpi, s_navPos.y + 14.0f * dpi));
        ImGui::BeginGroup();
        ctx.Icons().DrawIcon("layers", ImVec2(20.0f * dpi, 20.0f * dpi), colors.Accent);
        ImGui::SameLine(0.0f, 8.0f * dpi);
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
    const bool isCompact = isVertical && (s_navSize.x < 100.0f * dpi);

    ImVec2 size;
    if (isVertical) {
        if (isCompact) {
            size = ImVec2(s_navSize.x - 16.0f * dpi, 44.0f * dpi);
        } else {
            size = ImVec2((metrics.NavWidth - 16.0f) * dpi, 40.0f * dpi);
        }
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
    const float highlight = ctx.Anim().SpringFloat(id, targetHighlight, {140.0f, 20.0f, 1.0f, 0.001f});

    const float frameRounding = metrics.FrameRounding * dpi;

    // Render active/hover background pill
    if (highlight > 0.01f) {
        ImVec4 pillColor = selected ? colors.AccentMuted : colors.CardHover;
        pillColor.w *= highlight;
        drawList->AddRectFilled(bb.Min, bb.Max, ImColor(pillColor), frameRounding);

        if (selected) {
            if (isVertical) {
                float barX = (s_currentNavSide == NavSide::Left) ? bb.Min.x : (bb.Max.x - 3.0f * dpi);
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

    if (isCompact) {
        // Centered icon-only tab
        const float iconDim = 20.0f * dpi;
        const ImVec2 iconPos(bb.Min.x + (size.x - iconDim) * 0.5f, bb.Min.y + (size.y - iconDim) * 0.5f);
        if (sub.valid && sub.srv) {
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(sub.srv),
                iconPos,
                ImVec2(iconPos.x + iconDim, iconPos.y + iconDim),
                sub.uv0, sub.uv1,
                ImColor(iconTint)
            );
        }
        if (hovered) {
            ImGui::SetTooltip("%s", label);
        }
    } else {
        // Standard Icon + Label
        const float iconDim = 18.0f * dpi;
        const ImVec2 iconPos(bb.Min.x + 12.0f * dpi, bb.Min.y + (size.y - iconDim) * 0.5f);
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
        const ImVec2 textPos(iconPos.x + 28.0f * dpi, bb.Min.y + (size.y - textLineH) * 0.5f);
        drawList->AddText(textPos, ImColor(textCol), label);
        PopFont();

        if (badge && badge[0] != '\0') {
            PushFont(FontWeight::Caption);
            const ImVec2 badgeSize = ImGui::CalcTextSize(badge);
            const ImVec2 badgePos(bb.Max.x - badgeSize.x - 12.0f * dpi, bb.Min.y + (size.y - badgeSize.y) * 0.5f);
            drawList->AddRectFilled(
                ImVec2(badgePos.x - 5.0f * dpi, badgePos.y - 2.0f * dpi),
                ImVec2(badgePos.x + badgeSize.x + 5.0f * dpi, badgePos.y + badgeSize.y + 2.0f * dpi),
                ImColor(colors.AccentMuted),
                4.0f
            );
            drawList->AddText(badgePos, ImColor(colors.Accent), badge);
            PopFont();
        }
    }

    if (!isVertical) {
        ImGui::SameLine(0.0f, 6.0f * dpi);
    } else {
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 4.0f * dpi));
    }

    return pressed;
}

} // namespace parallax::layout
