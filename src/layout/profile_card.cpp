#include "parallax/layout/profile_card.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include <imgui_internal.h>
#include <algorithm>
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
    const float height = 34.0f * dpi;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, metrics.FrameRounding * dpi);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(32.0f * dpi, 7.0f * dpi));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.CardBg);

    char buffer[256];
    strncpy_s(buffer, sizeof(buffer), inOutQuery.c_str(), _TRUNCATE);

    ImGui::SetNextItemWidth(width);
    PushFont(FontWeight::Regular);
    bool changed = ImGui::InputTextWithHint("##ParallaxHeaderSearch", placeholder ? placeholder : "Search...", buffer, sizeof(buffer));
    PopFont();

    if (changed) {
        inOutQuery = buffer;
    }

    bool isFocused = ImGui::IsItemActive();
    float focusHighlight = ctx.Anim().SpringFloat(ImGui::GetID("##SearchFocus"), isFocused ? 1.0f : 0.0f, {150.0f, 20.0f, 1.0f});

    // Draw search icon inside the input
    const ImVec2 iconPos = ImVec2(bb.Min.x + 10.0f * dpi, bb.Min.y + (height - 16.0f * dpi) * 0.5f);
    const ImVec4 iconColor = isFocused ? colors.Accent : colors.TextMuted;
    ctx.Icons().DrawIcon(drawList, "search", iconPos, ImVec2(16.0f * dpi, 16.0f * dpi), iconColor);

    // Border highlight
    if (focusHighlight > 0.01f) {
        ImVec4 borderCol = colors.Accent;
        borderCol.w = focusHighlight;
        drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), metrics.FrameRounding * dpi, 0, 1.5f);
    } else {
        drawList->AddRect(bb.Min, bb.Max, ImColor(colors.Border), metrics.FrameRounding * dpi, 0, 1.0f);
    }

    // Clear 'X' button if text is present
    if (!inOutQuery.empty()) {
        const float clearDim = 16.0f * dpi;
        const ImVec2 clearBtnPos = ImVec2(bb.Max.x - clearDim - 10.0f * dpi, bb.Min.y + (height - clearDim) * 0.5f);
        ctx.Icons().DrawIcon(drawList, "x", clearBtnPos, ImVec2(clearDim, clearDim), colors.TextMuted);
        ImGui::SetCursorScreenPos(ImVec2(clearBtnPos.x - 2.0f, clearBtnPos.y - 2.0f));
        if (ImGui::InvisibleButton("##ClearSearch", ImVec2(clearDim + 4.0f, clearDim + 4.0f))) {
            inOutQuery.clear();
            changed = true;
        }
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    return changed;
}

void HeaderWidgets::ProfileCard(const UserProfile& profile, const ImVec2& size) {
    UserProfileCard(profile, size);
}

void UserProfileCard(const UserProfile& profile, const ImVec2& sizeArg) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 size = sizeArg;
    if (size.x <= 0.0f) {
        size.x = (metrics.NavWidth - 16.0f) * dpi;
    }
    if (size.y <= 0.0f) {
        size.y = metrics.ProfileCardHeight * dpi;
    }

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, ImGui::GetID("##UserProfileCard"))) return;

    const float rounding = metrics.FrameRounding * dpi;

    // Card background & 1px border
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(colors.CardBg), rounding);
    drawList->AddRect(bb.Min, bb.Max, ImColor(colors.Border), rounding, 0, 1.0f);

    const bool isCompact = (size.x < 100.0f * dpi);

    if (isCompact) {
        // Centered Avatar Circle in collapsed mode
        const float avatarRadius = 15.0f * dpi;
        const ImVec2 avatarCenter = ImVec2(bb.Min.x + size.x * 0.5f, bb.Min.y + size.y * 0.5f);
        drawList->AddCircleFilled(avatarCenter, avatarRadius, ImColor(colors.AccentMuted));
        drawList->AddCircle(avatarCenter, avatarRadius, ImColor(colors.Border), 24, 1.0f);

        // Icon inside Avatar
        const float iconDim = 15.0f * dpi;
        const char* iconName = profile.icon.empty() ? "user" : profile.icon.c_str();
        const ImVec2 iconPos = ImVec2(avatarCenter.x - iconDim * 0.5f, avatarCenter.y - iconDim * 0.5f);
        ctx.Icons().DrawIcon(drawList, iconName, iconPos, ImVec2(iconDim, iconDim), colors.TextPrimary);

        // Online Status Dot
        if (profile.isOnline) {
            const ImVec2 dotCenter = ImVec2(avatarCenter.x + avatarRadius * 0.70f, avatarCenter.y + avatarRadius * 0.70f);
            drawList->AddCircleFilled(dotCenter, 3.5f * dpi, ImColor(profile.statusColor));
            drawList->AddCircle(dotCenter, 3.5f * dpi, ImColor(colors.WindowBg), 12, 1.0f);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s\n%s", profile.username.c_str(), profile.subtitle.c_str());
        }
    } else {
        // Full expanded profile card
        const float avatarRadius = 18.0f * dpi;
        const ImVec2 avatarCenter = ImVec2(bb.Min.x + 12.0f * dpi + avatarRadius, bb.Min.y + size.y * 0.5f);
        drawList->AddCircleFilled(avatarCenter, avatarRadius, ImColor(colors.AccentMuted));
        drawList->AddCircle(avatarCenter, avatarRadius, ImColor(colors.Border), 24, 1.0f);

        // Icon inside Avatar
        const float iconDim = 18.0f * dpi;
        const char* iconName = profile.icon.empty() ? "user" : profile.icon.c_str();
        const ImVec2 iconPos = ImVec2(avatarCenter.x - iconDim * 0.5f, avatarCenter.y - iconDim * 0.5f);
        ctx.Icons().DrawIcon(drawList, iconName, iconPos, ImVec2(iconDim, iconDim), colors.TextPrimary);

        // Online Status Dot
        if (profile.isOnline) {
            const ImVec2 dotCenter = ImVec2(avatarCenter.x + avatarRadius * 0.70f, avatarCenter.y + avatarRadius * 0.70f);
            drawList->AddCircleFilled(dotCenter, 4.0f * dpi, ImColor(profile.statusColor));
            drawList->AddCircle(dotCenter, 4.0f * dpi, ImColor(colors.WindowBg), 12, 1.5f);
        }

        // Texts with typography hierarchy
        const float textStartX = avatarCenter.x + avatarRadius + 10.0f * dpi;
        const float textStartY = bb.Min.y + 11.0f * dpi;

        PushFont(FontWeight::SemiBold);
        drawList->AddText(ImVec2(textStartX, textStartY), ImColor(colors.TextPrimary), profile.username.c_str());
        PopFont();

        PushFont(FontWeight::Caption);
        drawList->AddText(ImVec2(textStartX, textStartY + 18.0f * dpi), ImColor(colors.TextMuted), profile.subtitle.c_str());
        PopFont();
    }
}

void HeaderWidgets::WindowSizeSelector(WindowSizeTier& inOutTier) {
    const char* names[] = {"Compact", "Standard", "Expanded"};
    int current = static_cast<int>(inOutTier);
    const auto& colors = StyleManager::Get().Colors();
    const float dpi = Context::Get().GetDpiScale();

    PushFont(FontWeight::Caption);
    for (int i = 0; i < 3; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 4.0f * dpi);

        const bool selected = (current == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, colors.AccentMuted);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.AccentMuted);
            ImGui::PushStyleColor(ImGuiCol_Text, colors.AccentHover);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Accent);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(20, 22, 32, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(30, 33, 48, 255));
            ImGui::PushStyleColor(ImGuiCol_Text, colors.TextSecondary);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * dpi);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * dpi, 4.0f * dpi));

        if (ImGui::Button(names[i])) {
            inOutTier = static_cast<WindowSizeTier>(i);
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);
    }
    PopFont();
}

void HeaderWidgets::NavSideSelector(NavSide& inOutSide) {
    const char* names[] = {"Left", "Right", "Top", "Bottom"};
    int current = static_cast<int>(inOutSide);
    const auto& colors = StyleManager::Get().Colors();
    const float dpi = Context::Get().GetDpiScale();

    PushFont(FontWeight::Caption);
    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 4.0f * dpi);

        const bool selected = (current == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, colors.AccentMuted);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.AccentMuted);
            ImGui::PushStyleColor(ImGuiCol_Text, colors.AccentHover);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Accent);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(20, 22, 32, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(30, 33, 48, 255));
            ImGui::PushStyleColor(ImGuiCol_Text, colors.TextSecondary);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * dpi);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * dpi, 4.0f * dpi));

        if (ImGui::Button(names[i])) {
            inOutSide = static_cast<NavSide>(i);
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);
    }
    PopFont();
}

} // namespace parallax::layout
