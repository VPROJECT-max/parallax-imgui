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

    const bool hasVisibleLabel = (label && label[0] && label[0] != '#' && label[1] != '#');
    if (hasVisibleLabel) {
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextSecondary, "%s", label);
        PopFont();
        ImGui::Spacing();
    }

    const float availWidth = ImGui::GetContentRegionAvail().x;
    const float width = (widthArg > 0.0f) ? widthArg : (widthArg < 0.0f ? availWidth : std::min(availWidth, 280.0f * dpi));
    const float height = 36.0f * dpi;

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect frameBb(pos, ImVec2(pos.x + width, pos.y + height));
    const ImGuiID id = window->GetID(label);

    ImGui::ItemSize(frameBb, g.Style.FramePadding.y);
    if (!ImGui::ItemAdd(frameBb, id, &frameBb)) return false;

    const bool hovered = ImGui::ItemHoverable(frameBb, id, g.CurrentItemFlags);
    const bool isFocused = (g.ActiveId == id);

    // Spring focus border animation
    char focusIdStr[64];
    snprintf(focusIdStr, sizeof(focusIdStr), "##focus_%08X", id);
    const float targetFocus = isFocused ? 1.0f : (hovered ? 0.35f : 0.0f);
    const float animFocus = ctx.Anim().SpringFloat(ImGui::GetID(focusIdStr), targetFocus, {140.0f, 20.0f, 1.0f, 0.001f});

    ImDrawList* drawList = window->DrawList;
    const float rounding = metrics.FrameRounding * dpi;

    // Draw custom background
    const ImU32 bgCol = IM_COL32(15, 17, 26, 255);
    drawList->AddRectFilled(frameBb.Min, frameBb.Max, bgCol, rounding);

    // Draw animated focus glow and border
    if (animFocus > 0.05f) {
        drawList->AddRect(
            ImVec2(frameBb.Min.x - 1.0f, frameBb.Min.y - 1.0f),
            ImVec2(frameBb.Max.x + 1.0f, frameBb.Max.y + 1.0f),
            ImColor(colors.Accent.x, colors.Accent.y, colors.Accent.z, 0.25f * animFocus),
            rounding + 1.0f,
            0,
            2.0f
        );
    }

    const ImVec4 borderVec(
        colors.Border.x + (colors.Accent.x - colors.Border.x) * animFocus,
        colors.Border.y + (colors.Accent.y - colors.Border.y) * animFocus,
        colors.Border.z + (colors.Accent.z - colors.Border.z) * animFocus,
        0.10f + 0.65f * animFocus
    );
    drawList->AddRect(frameBb.Min, frameBb.Max, ImColor(borderVec), rounding, 0, 1.0f);

    float textStartX = frameBb.Min.x + 12.0f * dpi;

    // Render optional leading icon
    if (icon && icon[0]) {
        const float iconDim = 16.0f * dpi;
        const ImVec2 iconPos(textStartX, frameBb.Min.y + (height - iconDim) * 0.5f);
        ImGui::SetCursorScreenPos(iconPos);

        const ImVec4 iconTint = isFocused ? colors.Accent : colors.TextMuted;
        ctx.Icons().DrawIcon(icon, ImVec2(iconDim, iconDim), iconTint);
        textStartX += iconDim + 8.0f * dpi;
    }

    // Clear button if text is not empty and hovered/focused
    const bool hasText = (buf[0] != '\0');
    float textEndX = frameBb.Max.x - 10.0f * dpi;
    if (hasText && (hovered || isFocused)) {
        const float clearSize = 16.0f * dpi;
        const ImVec2 clearPos(frameBb.Max.x - clearSize - 8.0f * dpi, frameBb.Min.y + (height - clearSize) * 0.5f);
        const ImRect clearBb(clearPos, ImVec2(clearPos.x + clearSize, clearPos.y + clearSize));

        char clearIdStr[64];
        snprintf(clearIdStr, sizeof(clearIdStr), "##clear_%08X", id);
        const ImGuiID clearId = ImGui::GetID(clearIdStr);
        bool clearHovered = false, clearHeld = false;
        bool clearPressed = ImGui::ButtonBehavior(clearBb, clearId, &clearHovered, &clearHeld);

        drawList->AddCircleFilled(
            ImVec2(clearPos.x + clearSize * 0.5f, clearPos.y + clearSize * 0.5f),
            clearSize * 0.45f,
            clearHovered ? IM_COL32(255, 255, 255, 40) : IM_COL32(255, 255, 255, 20)
        );

        const float pad = 4.0f * dpi;
        const ImU32 crossCol = clearHovered ? IM_COL32(255, 255, 255, 220) : IM_COL32(160, 165, 180, 180);
        drawList->AddLine(ImVec2(clearBb.Min.x + pad, clearBb.Min.y + pad), ImVec2(clearBb.Max.x - pad, clearBb.Max.y - pad), crossCol, 1.5f);
        drawList->AddLine(ImVec2(clearBb.Max.x - pad, clearBb.Min.y + pad), ImVec2(clearBb.Min.x + pad, clearBb.Max.y - pad), crossCol, 1.5f);

        if (clearPressed) {
            buf[0] = '\0';
        }
        textEndX = clearPos.x - 4.0f * dpi;
    }

    // Set cursor for native text input inside custom frame
    const float inputWidth = textEndX - textStartX;
    ImGui::SetCursorScreenPos(ImVec2(textStartX, frameBb.Min.y + (height - ImGui::GetTextLineHeight()) * 0.5f - 1.0f));

    ImGui::PushItemWidth(inputWidth);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, colors.TextPrimary);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    if (isPassword) flags |= ImGuiInputTextFlags_Password;

    PushFont(FontWeight::Regular);
    bool valueChanged = ImGui::InputText("##InnerCustomInput", buf, bufSize, flags);

    // Draw placeholder if text is empty and not focused
    if (!hasText && !isFocused && placeholder && placeholder[0]) {
        drawList->AddText(
            ImVec2(textStartX, frameBb.Min.y + (height - ImGui::GetTextLineHeight()) * 0.5f),
            ImColor(colors.TextMuted),
            placeholder
        );
    }

    PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();

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
