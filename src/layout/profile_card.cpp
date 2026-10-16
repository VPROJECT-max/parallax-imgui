#include "parallax/layout/profile_card.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include "parallax/widgets/color_picker.hpp"
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
    ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.WidgetBg);
    ImGui::PushStyleColor(ImGuiCol_Text, colors.TextPrimary);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, colors.TextMuted);

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
    const ImVec2 iconPos = ImVec2(bb.Min.x + 10.0f * dpi, bb.Min.y + (height - 15.0f * dpi) * 0.5f);
    const ImVec4 iconColor = isFocused ? colors.Accent : colors.TextMuted;
    ctx.Icons().DrawIcon(drawList, "search", iconPos, ImVec2(15.0f * dpi, 15.0f * dpi), iconColor);

    // Border highlight
    if (focusHighlight > 0.01f) {
        ImVec4 borderCol = colors.Accent;
        borderCol.w = focusHighlight;
        drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), metrics.FrameRounding * dpi, 0, 1.25f);
    } else {
        drawList->AddRect(bb.Min, bb.Max, ImColor(colors.Border), metrics.FrameRounding * dpi, 0, 1.0f);
    }

    // Clear 'X' button if text is present
    if (!inOutQuery.empty()) {
        const float clearDim = 14.0f * dpi;
        const ImVec2 clearBtnPos = ImVec2(bb.Max.x - clearDim - 10.0f * dpi, bb.Min.y + (height - clearDim) * 0.5f);
        ctx.Icons().DrawIcon(drawList, "x", clearBtnPos, ImVec2(clearDim, clearDim), colors.TextMuted);
        const ImVec2 restorePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(clearBtnPos.x - 2.0f, clearBtnPos.y - 2.0f));
        if (ImGui::InvisibleButton("##ClearSearch", ImVec2(clearDim + 4.0f, clearDim + 4.0f))) {
            inOutQuery.clear();
            changed = true;
        }
        ImGui::SetCursorScreenPos(restorePos);
    }

    ImGui::PopStyleColor(3);
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
        ctx.Icons().DrawIcon(drawList, iconName, iconPos, ImVec2(iconDim, iconDim), colors.Accent);

        // Online Status Dot
        if (profile.isOnline) {
            const ImVec2 dotCenter = ImVec2(avatarCenter.x + avatarRadius * 0.70f, avatarCenter.y + avatarRadius * 0.70f);
            drawList->AddCircleFilled(dotCenter, 3.5f * dpi, ImColor(colors.Success));
            drawList->AddCircle(dotCenter, 3.5f * dpi, ImColor(colors.CardBg), 12, 1.0f);
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
        ctx.Icons().DrawIcon(drawList, iconName, iconPos, ImVec2(iconDim, iconDim), colors.Accent);

        // Online Status Dot
        if (profile.isOnline) {
            const ImVec2 dotCenter = ImVec2(avatarCenter.x + avatarRadius * 0.70f, avatarCenter.y + avatarRadius * 0.70f);
            drawList->AddCircleFilled(dotCenter, 4.0f * dpi, ImColor(colors.Success));
            drawList->AddCircle(dotCenter, 4.0f * dpi, ImColor(colors.CardBg), 12, 1.5f);
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
            ImGui::PushStyleColor(ImGuiCol_Text, colors.Accent);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Accent);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, colors.WidgetBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.WidgetHover);
            ImGui::PushStyleColor(ImGuiCol_Text, colors.TextSecondary);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f * dpi);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f * dpi, 5.0f * dpi));

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
            ImGui::PushStyleColor(ImGuiCol_Text, colors.Accent);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Accent);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, colors.WidgetBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.WidgetHover);
            ImGui::PushStyleColor(ImGuiCol_Text, colors.TextSecondary);
            ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f * dpi);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f * dpi, 5.0f * dpi));

        if (ImGui::Button(names[i])) {
            inOutSide = static_cast<NavSide>(i);
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);
    }
    PopFont();
}

bool HeaderWidgets::ThemeSwitcher(const char* label) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    auto& styleMgr = StyleManager::Get();
    const auto& colors = styleMgr.Colors();
    const auto& metrics = styleMgr.Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (label && label[0] != '\0' && !(label[0] == '#' && label[1] == '#')) {
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextSecondary, "%s", label);
        PopFont();
        ImGui::Spacing();
    }

    const ImGuiID id = window->GetID(label ? label : "##ParallaxThemeSwitcher");
    const float availW = ImGui::GetContentRegionAvail().x;
    const float totalW = (availW > 120.0f * dpi) ? availW : 240.0f * dpi;
    const float totalH = 34.0f * dpi;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + totalW, pos.y + totalH));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    bool changed = false;

    const float halfW = totalW * 0.5f;
    const float mouseX = ImGui::GetIO().MousePos.x;

    if (pressed) {
        ThemeMode targetMode = (mouseX >= bb.Min.x + halfW) ? ThemeMode::Light : ThemeMode::Dark;
        if (targetMode == styleMgr.GetTheme()) {
            // Clicking anywhere toggles if they clicked the already-active half
            styleMgr.ToggleTheme();
        } else {
            styleMgr.SetTheme(targetMode);
        }
        changed = true;
    }

    const bool isLight = styleMgr.IsLightTheme();
    const float slideT = ctx.Anim().SpringFloat(id ^ 0x71E3Eu, isLight ? 1.0f : 0.0f, {240.0f, 24.0f, 1.0f});
    const float rounding = metrics.FrameRounding * dpi;

    // Outer recessed track
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(colors.WidgetBg), rounding);
    drawList->AddRect(bb.Min, bb.Max, ImColor(hovered ? colors.BorderStrong : colors.Border), rounding, 0, 1.0f);

    // Animated sliding active pill
    const float pad = 3.0f * dpi;
    const float pillW = halfW - pad * 1.5f;
    const float pillX = bb.Min.x + pad + slideT * (totalW - pillW - pad * 2.0f);
    const ImVec2 pillMin(pillX, bb.Min.y + pad);
    const ImVec2 pillMax(pillX + pillW, bb.Max.y - pad);

    drawList->AddRectFilled(pillMin, pillMax, ImColor(colors.AccentMuted), rounding - 1.5f * dpi);
    drawList->AddRect(pillMin, pillMax, ImColor(colors.Accent), rounding - 1.5f * dpi, 0, 1.0f);

    // Left Option: Graphite (Dark)
    {
        PushFont(FontWeight::Medium);
        const char* darkText = "Graphite (Dark)";
        const ImVec2 tSize = ImGui::CalcTextSize(darkText);
        const float iconDim = 14.0f * dpi;
        const float groupW = iconDim + 6.0f * dpi + tSize.x;
        const float startX = bb.Min.x + (halfW - groupW) * 0.5f;
        const ImVec4 itemCol = !isLight ? colors.TextPrimary : colors.TextSecondary;
        const ImVec4 icCol = !isLight ? colors.Accent : colors.TextMuted;

        ctx.Icons().DrawIcon(
            drawList, "moon",
            ImVec2(startX, bb.Min.y + (totalH - iconDim) * 0.5f),
            ImVec2(iconDim, iconDim),
            icCol
        );
        drawList->AddText(
            ImVec2(startX + iconDim + 6.0f * dpi, bb.Min.y + (totalH - tSize.y) * 0.5f),
            ImColor(itemCol),
            darkText
        );
        PopFont();
    }

    // Right Option: Porcelain (Light)
    {
        PushFont(FontWeight::Medium);
        const char* lightText = "Porcelain (Light)";
        const ImVec2 tSize = ImGui::CalcTextSize(lightText);
        const float iconDim = 14.0f * dpi;
        const float groupW = iconDim + 6.0f * dpi + tSize.x;
        const float startX = bb.Min.x + halfW + (halfW - groupW) * 0.5f;
        const ImVec4 itemCol = isLight ? colors.TextPrimary : colors.TextSecondary;
        const ImVec4 icCol = isLight ? colors.Accent : colors.TextMuted;

        ctx.Icons().DrawIcon(
            drawList, "sun",
            ImVec2(startX, bb.Min.y + (totalH - iconDim) * 0.5f),
            ImVec2(iconDim, iconDim),
            icCol
        );
        drawList->AddText(
            ImVec2(startX + iconDim + 6.0f * dpi, bb.Min.y + (totalH - tSize.y) * 0.5f),
            ImColor(itemCol),
            lightText
        );
        PopFont();
    }

    return changed;
}

bool HeaderWidgets::AccentSelector(const char* label, bool showCustomPicker) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    auto& styleMgr = StyleManager::Get();
    const auto& colors = styleMgr.Colors();
    const auto& metrics = styleMgr.Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (label && label[0] != '\0' && !(label[0] == '#' && label[1] == '#')) {
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextSecondary, "%s", label);
        PopFont();
        ImGui::Spacing();
    }

    struct PresetDef {
        AccentPreset preset;
        const char* name;
        ImVec4 previewColor;
    };

    const bool isLight = styleMgr.IsLightTheme();
    const PresetDef presets[] = {
        { AccentPreset::Sage,       "Sage",       isLight ? ImVec4(0.32f, 0.52f, 0.42f, 1.0f) : ImVec4(0.471f, 0.631f, 0.545f, 1.0f) },
        { AccentPreset::Monochrome, "Graphite",   isLight ? ImVec4(0.24f, 0.26f, 0.30f, 1.0f) : ImVec4(0.76f, 0.78f, 0.82f, 1.0f)  },
        { AccentPreset::Copper,     "Copper",     isLight ? ImVec4(0.68f, 0.42f, 0.30f, 1.0f) : ImVec4(0.769f, 0.541f, 0.416f, 1.0f) },
        { AccentPreset::SteelBlue,  "Steel Blue", isLight ? ImVec4(0.29f, 0.45f, 0.60f, 1.0f) : ImVec4(0.431f, 0.569f, 0.690f, 1.0f) }
    };

    bool changed = false;
    const AccentPreset currentPreset = styleMgr.GetAccentPreset();
    const float availW = ImGui::GetContentRegionAvail().x;
    const float gap = 6.0f * dpi;
    const float pillW = std::max(60.0f * dpi, (availW - gap * 3.0f) * 0.25f);
    const float pillH = 30.0f * dpi;
    const float rounding = metrics.FrameRounding * dpi;

    PushFont(FontWeight::Caption);
    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, gap);

        const ImVec2 pPos = ImGui::GetCursorScreenPos();
        const ImRect pBb(pPos, ImVec2(pPos.x + pillW, pPos.y + pillH));
        char btnIdStr[64];
        snprintf(btnIdStr, sizeof(btnIdStr), "##AccentPresetBtn_%d", i);
        const ImGuiID pId = window->GetID(btnIdStr);

        ImGui::ItemSize(pBb);
        if (!ImGui::ItemAdd(pBb, pId)) continue;

        bool pHov = false, pHeld = false;
        if (ImGui::ButtonBehavior(pBb, pId, &pHov, &pHeld)) {
            styleMgr.SetAccentPreset(presets[i].preset);
            changed = true;
        }

        const bool isSelected = (currentPreset == presets[i].preset);
        const float selAnim = ctx.Anim().SpringFloat(pId, isSelected ? 1.0f : (pHov ? 0.35f : 0.0f), {220.0f, 22.0f, 1.0f});

        ImVec4 bgCol = isSelected ? colors.AccentMuted : (pHov ? colors.WidgetHover : colors.WidgetBg);
        drawList->AddRectFilled(pBb.Min, pBb.Max, ImColor(bgCol), rounding);
        drawList->AddRect(
            pBb.Min, pBb.Max,
            ImColor(isSelected ? colors.Accent : (pHov ? colors.BorderStrong : colors.Border)),
            rounding, 0, isSelected ? 1.25f : 1.0f
        );
        (void)selAnim;

        // Swatch dot
        const float dotR = 5.0f * dpi;
        const ImVec2 dotCenter(pBb.Min.x + 12.0f * dpi, pBb.Min.y + pillH * 0.5f);
        drawList->AddCircleFilled(dotCenter, dotR, ImColor(presets[i].previewColor), 16);
        drawList->AddCircle(dotCenter, dotR, ImColor(colors.BorderStrong), 16, 1.0f);

        // Preset label
        const float th = ImGui::GetFontSize();
        drawList->AddText(
            ImVec2(dotCenter.x + dotR + 6.0f * dpi, pBb.Min.y + (pillH - th) * 0.5f),
            ImColor(isSelected ? colors.TextPrimary : colors.TextSecondary),
            presets[i].name
        );
    }
    PopFont();

    if (showCustomPicker) {
        ImGui::Spacing();
        ImVec4 customCol = styleMgr.GetCustomAccent();
        if (ColorPickerRow("Custom Accent Color", &customCol, false)) {
            styleMgr.SetCustomAccent(customCol);
            changed = true;
        }
    }

    return changed;
}

bool ThemeSwitcher(const char* label) {
    return HeaderWidgets::ThemeSwitcher(label);
}

bool AccentSelector(const char* label, bool showCustomPicker) {
    return HeaderWidgets::AccentSelector(label, showCustomPicker);
}

} // namespace parallax::layout
