#include "parallax/widgets/input_text.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <cstdio>

namespace parallax {

bool InputText(
    const char* label,
    char* buf,
    size_t bufSize,
    const char* icon,
    const char* placeholder,
    float widthArg,
    bool isPassword
) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();

    const bool hasVisibleLabel = (label && label[0] && !(label[0] == '#' && label[1] == '#'));
    if (hasVisibleLabel) {
        PushFont(FontWeight::Medium);
        const char* labelEnd = ImGui::FindRenderedTextEnd(label);
        if (labelEnd > label) {
            ImGui::TextColored(colors.TextSecondary, "%.*s", static_cast<int>(labelEnd - label), label);
            ImGui::Dummy(ImVec2(0.0f, 1.0f * dpi));
        }
        PopFont();
    }

    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float width = (widthArg > 0.0f) ? widthArg : availWidth;
    const float height = 32.0f * dpi;

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect frameBb(pos, ImVec2(pos.x + width, pos.y + height));
    const ImGuiID id = window->GetID(label);

    ImGui::ItemSize(frameBb, g.Style.FramePadding.y);
    if (!ImGui::ItemAdd(frameBb, id, &frameBb)) return false;

    const ImVec2 nextCursorPos = ImGui::GetCursorScreenPos();

    char innerIdStr[64];
    snprintf(innerIdStr, sizeof(innerIdStr), "##InnerInput_%08X", id);
    const ImGuiID innerId = window->GetID(innerIdStr);

    const bool hovered = ImGui::ItemHoverable(frameBb, id, g.CurrentItemFlags) ||
                         frameBb.Contains(ImGui::GetIO().MousePos);
    const bool isFocused = (g.ActiveId == innerId);

    const float targetFocus = isFocused ? 1.0f : (hovered ? 0.35f : 0.0f);
    const float animFocus = ctx.Anim().SpringFloat(id + 0x6100, targetFocus, {180.0f, 22.0f, 1.0f, 0.001f});

    ImDrawList* drawList = window->DrawList;
    const float rounding = metrics.FrameRounding * dpi;

    // Theme-aware interactive control surface
    const ImVec4 bgCol(
        colors.WidgetBg.x + (colors.WidgetHover.x - colors.WidgetBg.x) * (hovered ? 0.6f : 0.0f),
        colors.WidgetBg.y + (colors.WidgetHover.y - colors.WidgetBg.y) * (hovered ? 0.6f : 0.0f),
        colors.WidgetBg.z + (colors.WidgetHover.z - colors.WidgetBg.z) * (hovered ? 0.6f : 0.0f),
        1.0f
    );
    drawList->AddRectFilled(frameBb.Min, frameBb.Max, ImColor(bgCol), rounding);

    // Animated focus halo & border
    if (animFocus > 0.05f) {
        ImVec4 halo = colors.AccentMuted;
        halo.w *= 0.65f * animFocus;
        drawList->AddRect(
            ImVec2(frameBb.Min.x - 1.5f * dpi, frameBb.Min.y - 1.5f * dpi),
            ImVec2(frameBb.Max.x + 1.5f * dpi, frameBb.Max.y + 1.5f * dpi),
            ImColor(halo),
            rounding + 1.5f * dpi,
            0,
            1.5f * dpi
        );
    }

    const ImVec4 borderVec(
        colors.Border.x + (colors.Accent.x - colors.Border.x) * animFocus,
        colors.Border.y + (colors.Accent.y - colors.Border.y) * animFocus,
        colors.Border.z + (colors.Accent.z - colors.Border.z) * animFocus,
        colors.Border.w + (colors.Accent.w - colors.Border.w) * animFocus
    );
    drawList->AddRect(frameBb.Min, frameBb.Max, ImColor(borderVec), rounding, 0, isFocused ? 1.5f : 1.0f);

    float textStartX = frameBb.Min.x + 10.0f * dpi;

    // Optional leading icon
    if (icon && icon[0]) {
        const float iconDim = 15.0f * dpi;
        const ImVec2 iconPos(textStartX, frameBb.Min.y + (height - iconDim) * 0.5f);
        const ImVec4 iconTint = isFocused ? colors.Accent : colors.TextSecondary;
        ctx.Icons().DrawIcon(drawList, icon, iconPos, ImVec2(iconDim, iconDim), iconTint);
        textStartX += iconDim + 8.0f * dpi;
    }

    // Theme-aware Clear Button if text is not empty and hovered/focused
    const bool hasText = (buf[0] != '\0');
    float textEndX = frameBb.Max.x - 10.0f * dpi;
    bool clearedThisFrame = false;
    if (hasText && (hovered || isFocused)) {
        const float clearSize = 16.0f * dpi;
        const ImVec2 clearPos(frameBb.Max.x - clearSize - 8.0f * dpi, frameBb.Min.y + (height - clearSize) * 0.5f);
        const ImRect clearBb(clearPos, ImVec2(clearPos.x + clearSize, clearPos.y + clearSize));

        char clearIdStr[64];
        snprintf(clearIdStr, sizeof(clearIdStr), "##clear_%08X", id);
        const ImGuiID clearId = window->GetID(clearIdStr);
        bool clearHovered = false, clearHeld = false;
        bool clearPressed = ImGui::ButtonBehavior(clearBb, clearId, &clearHovered, &clearHeld);

        ImVec4 pillCol = clearHovered ? colors.AccentMuted : colors.CardHover;
        drawList->AddCircleFilled(
            ImVec2(clearPos.x + clearSize * 0.5f, clearPos.y + clearSize * 0.5f),
            clearSize * 0.48f,
            ImColor(pillCol)
        );

        const float pad = 4.5f * dpi;
        const ImU32 crossCol = ImColor(clearHovered ? colors.Accent : colors.TextSecondary);
        drawList->AddLine(ImVec2(clearBb.Min.x + pad, clearBb.Min.y + pad), ImVec2(clearBb.Max.x - pad, clearBb.Max.y - pad), crossCol, 1.5f * dpi);
        drawList->AddLine(ImVec2(clearBb.Max.x - pad, clearBb.Min.y + pad), ImVec2(clearBb.Min.x + pad, clearBb.Max.y - pad), crossCol, 1.5f * dpi);

        if (clearPressed) {
            buf[0] = '\0';
            clearedThisFrame = true;
        }
        textEndX = clearPos.x - 4.0f * dpi;
    }

    const float inputWidth = std::max(textEndX - textStartX, 10.0f * dpi);
    ImGui::SetCursorScreenPos(ImVec2(textStartX, frameBb.Min.y + (height - ImGui::GetTextLineHeight()) * 0.5f));

    ImGui::PushItemWidth(inputWidth);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, colors.TextPrimary);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    if (isPassword) flags |= ImGuiInputTextFlags_Password;

    PushFont(FontWeight::Regular);
    bool valueChanged = ImGui::InputText(innerIdStr, buf, bufSize, flags) || clearedThisFrame;

    if (buf[0] == '\0' && !ImGui::IsItemActive() && placeholder && placeholder[0]) {
        drawList->AddText(
            ImVec2(textStartX, frameBb.Min.y + (height - ImGui::GetTextLineHeight()) * 0.5f),
            ImColor(colors.TextMuted),
            placeholder
        );
    }

    PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(5);
    ImGui::PopItemWidth();

    ImGui::SetCursorScreenPos(nextCursorPos);
    return valueChanged;
}

bool InputText(
    const char* label,
    std::string& text,
    const char* icon,
    const char* placeholder,
    float width,
    bool isPassword
) {
    char buf[1024];
    strncpy_s(buf, sizeof(buf), text.c_str(), _TRUNCATE);
    bool changed = InputText(label, buf, sizeof(buf), icon, placeholder, width, isPassword);
    if (changed) {
        text = buf;
    }
    return changed;
}

} // namespace parallax
