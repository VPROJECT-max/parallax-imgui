#include "parallax/layout/profile_card.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <vector>

namespace parallax::layout {

bool HeaderWidgets::SearchBar(std::string& inOutQuery, const char* placeholder, float width) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (width <= 0.0f) {
        width = metrics.SearchBarWidth * dpi;
    }
    const float height = 32.0f * dpi;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, metrics.FrameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(28.0f * dpi, 6.0f * dpi));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.CardBg);

    char buffer[256];
    strncpy_s(buffer, sizeof(buffer), inOutQuery.c_str(), _TRUNCATE);

    ImGui::SetNextItemWidth(width);
    bool changed = ImGui::InputTextWithHint("##ParallaxHeaderSearch", placeholder, buffer, sizeof(buffer));
    if (changed) {
        inOutQuery = buffer;
    }

    bool isFocused = ImGui::IsItemActive();
    float focusHighlight = ctx.Anim().SpringFloat(ImGui::GetID("##SearchFocus"), isFocused ? 1.0f : 0.0f, {150.0f, 20.0f, 1.0f});

    // Draw search icon inside the input
    ImVec2 iconPos = ImVec2(bb.Min.x + 8.0f * dpi, bb.Min.y + (height - 16.0f * dpi) * 0.5f);
    ImVec4 iconColor = isFocused ? colors.Accent : colors.TextMuted;
    auto sub = ctx.Icons().GetIcon("search");
    if (sub.valid && sub.srv) {
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(sub.srv),
            iconPos,
            ImVec2(iconPos.x + 16.0f * dpi, iconPos.y + 16.0f * dpi),
            sub.uv0, sub.uv1,
            ImColor(iconColor)
        );
    }

    // Border highlight
    ImVec4 borderCol = colors.Border;
    if (focusHighlight > 0.01f) {
        borderCol = colors.Accent;
        borderCol.w = focusHighlight;
        drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), metrics.FrameRounding, 0, 1.5f);
    }

    // Clear 'X' button if text is present
    if (!inOutQuery.empty()) {
        ImVec2 clearBtnPos = ImVec2(bb.Max.x - 22.0f * dpi, bb.Min.y + (height - 14.0f * dpi) * 0.5f);
        auto clearSub = ctx.Icons().GetIcon("x");
        if (clearSub.valid && clearSub.srv) {
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(clearSub.srv),
                clearBtnPos,
                ImVec2(clearBtnPos.x + 14.0f * dpi, clearBtnPos.y + 14.0f * dpi),
                clearSub.uv0, clearSub.uv1,
                ImColor(colors.TextMuted)
            );
        }
        ImGui::SetCursorScreenPos(ImVec2(clearBtnPos.x - 2.0f, clearBtnPos.y - 2.0f));
        if (ImGui::InvisibleButton("##ClearSearch", ImVec2(18.0f * dpi, 18.0f * dpi))) {
            inOutQuery.clear();
            changed = true;
        }
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    return changed;
}

void HeaderWidgets::ProfileCard(const UserProfile& profile, const ImVec2& size) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 cardSize = size;
    if (cardSize.x <= 0.0f) cardSize.x = (metrics.NavWidth - 16.0f) * dpi;
    if (cardSize.y <= 0.0f) cardSize.y = metrics.ProfileCardHeight * dpi;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImRect bb(pos, ImVec2(pos.x + cardSize.x, pos.y + cardSize.y));

    ImGui::ItemSize(bb);
    ImGuiID id = ImGui::GetID("##UserProfileCard");
    if (!ImGui::ItemAdd(bb, id)) return;

    bool hovered = false;
    bool held = false;
    ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Hover background
    if (hovered) {
        drawList->AddRectFilled(bb.Min, bb.Max, ImColor(colors.CardHover), metrics.FrameRounding);
    }
    drawList->AddRect(bb.Min, bb.Max, ImColor(colors.Border), metrics.FrameRounding);

    // Avatar Circle & Icon
    const float avatarRadius = 16.0f * dpi;
    const ImVec2 avatarCenter = ImVec2(bb.Min.x + 22.0f * dpi, bb.Min.y + cardSize.y * 0.5f);
    drawList->AddCircleFilled(avatarCenter, avatarRadius, ImColor(colors.HeaderBg));
    drawList->AddCircle(avatarCenter, avatarRadius, ImColor(colors.Border), 24, 1.0f);

    auto sub = ctx.Icons().GetIcon(profile.icon.empty() ? "user" : profile.icon);
    if (sub.valid && sub.srv) {
        const float iconDim = 18.0f * dpi;
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(sub.srv),
            ImVec2(avatarCenter.x - iconDim * 0.5f, avatarCenter.y - iconDim * 0.5f),
            ImVec2(avatarCenter.x + iconDim * 0.5f, avatarCenter.y + iconDim * 0.5f),
            sub.uv0, sub.uv1,
            ImColor(colors.TextPrimary)
        );
    }

    // Online Status Dot
    if (profile.isOnline) {
        const ImVec2 dotCenter = ImVec2(avatarCenter.x + avatarRadius * 0.7f, avatarCenter.y + avatarRadius * 0.7f);
        drawList->AddCircleFilled(dotCenter, 4.0f * dpi, ImColor(profile.statusColor));
        drawList->AddCircle(dotCenter, 4.0f * dpi, ImColor(colors.WindowBg), 12, 1.5f);
    }

    // Texts
    const ImVec2 textPos = ImVec2(avatarCenter.x + avatarRadius + 8.0f * dpi, bb.Min.y + 10.0f * dpi);
    drawList->AddText(textPos, ImColor(colors.TextPrimary), profile.username.c_str());

    const ImVec2 subtextPos = ImVec2(textPos.x, textPos.y + ImGui::GetFontSize() + 2.0f * dpi);
    drawList->AddText(subtextPos, ImColor(colors.TextMuted), profile.subtitle.c_str());
}

void HeaderWidgets::WindowSizeSelector(WindowSizeTier& inOutTier) {
    const char* names[] = {"Compact", "Standard", "Expanded"};
    int current = static_cast<int>(inOutTier);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    for (int i = 0; i < 3; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 4.0f);

        bool selected = (current == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, StyleManager::Get().Colors().AccentMuted);
            ImGui::PushStyleColor(ImGuiCol_Text, StyleManager::Get().Colors().Accent);
        }

        if (ImGui::Button(names[i])) {
            inOutTier = static_cast<WindowSizeTier>(i);
        }

        if (selected) {
            ImGui::PopStyleColor(2);
        }
    }
    ImGui::PopStyleVar();
}

void HeaderWidgets::NavSideSelector(NavSide& inOutSide) {
    const char* names[] = {"Left", "Right", "Top", "Bottom"};
    int current = static_cast<int>(inOutSide);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 4.0f);

        bool selected = (current == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, StyleManager::Get().Colors().AccentMuted);
            ImGui::PushStyleColor(ImGuiCol_Text, StyleManager::Get().Colors().Accent);
        }

        if (ImGui::Button(names[i])) {
            inOutSide = static_cast<NavSide>(i);
        }

        if (selected) {
            ImGui::PopStyleColor(2);
        }
    }
    ImGui::PopStyleVar();
}

} // namespace parallax::layout
