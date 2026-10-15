#include "parallax/widgets/checkbox.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace parallax {

bool ToggleEx(const char* label, bool* v, const ToggleOptions& options) {
    if (!v) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);
    const float availW = ImGui::GetContentRegionAvail().x;

    const bool hasDesc = (options.description && options.description[0] != '\0');
    const float rowHeight = hasDesc ? (38.0f * dpi) : (26.0f * dpi);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowHeight));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = false;

    if (!options.disabled) {
        pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    }

    if (pressed) {
        *v = !(*v);
        if (options.onChange) {
            options.onChange(*v);
        }
    }

    // Animated check progress (0.0 to 1.0)
    const float targetCheck = *v ? 1.0f : 0.0f;
    const float checkProgress = ctx.Anim().SpringFloat(id, targetCheck, {200.0f, 22.0f, 1.0f});

    // Row hover background highlight
    if (hovered && !options.disabled) {
        drawList->AddRectFilled(
            ImVec2(bb.Min.x - 4.0f * dpi, bb.Min.y - 2.0f * dpi),
            ImVec2(bb.Max.x + 4.0f * dpi, bb.Max.y + 2.0f * dpi),
            ImColor(colors.CardHover),
            metrics.FrameRounding
        );
    }

    // Left side: Icon + Label + Description
    float textCursorX = bb.Min.x + 4.0f * dpi;

    if (options.icon && options.icon[0] != '\0') {
        const float iconDim = 16.0f * dpi;
        const ImVec2 iconPos = ImVec2(textCursorX, bb.Min.y + (hasDesc ? 6.0f * dpi : (rowHeight - iconDim) * 0.5f));
        auto sub = ctx.Icons().GetIcon(options.icon);
        if (sub.valid && sub.srv) {
            ImVec4 iconCol = options.disabled ? colors.TextMuted : (hovered ? colors.Accent : colors.TextSecondary);
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(sub.srv),
                iconPos,
                ImVec2(iconPos.x + iconDim, iconPos.y + iconDim),
                sub.uv0, sub.uv1,
                ImColor(iconCol)
            );
        }
        textCursorX += iconDim + 8.0f * dpi;
    }

    ImVec4 labelCol = options.disabled ? colors.TextMuted : colors.TextPrimary;
    const float labelY = hasDesc ? (bb.Min.y + 4.0f * dpi) : (bb.Min.y + (rowHeight - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(ImVec2(textCursorX, labelY), ImColor(labelCol), label);

    if (hasDesc) {
        const float descY = labelY + ImGui::GetFontSize() + 2.0f * dpi;
        drawList->AddText(ImVec2(textCursorX, descY), ImColor(colors.TextMuted), options.description);
    }

    // Right side: Interactive Switch / Checkbox
    if (options.style == ToggleStyle::PillSwitch) {
        // Pill Switch dimensions
        const float switchW = 34.0f * dpi;
        const float switchH = 18.0f * dpi;
        const float switchY = bb.Min.y + (rowHeight - switchH) * 0.5f;
        const ImVec2 switchMin = ImVec2(bb.Max.x - switchW - 4.0f * dpi, switchY);
        const ImVec2 switchMax = ImVec2(switchMin.x + switchW, switchMin.y + switchH);

        // Smooth color blend from inactive card to active accent
        ImVec4 pillBg = ImVec4(
            colors.CardBg.x + (colors.Accent.x - colors.CardBg.x) * checkProgress,
            colors.CardBg.y + (colors.Accent.y - colors.CardBg.y) * checkProgress,
            colors.CardBg.z + (colors.Accent.z - colors.CardBg.z) * checkProgress,
            options.disabled ? 0.35f : 1.0f
        );

        drawList->AddRectFilled(switchMin, switchMax, ImColor(pillBg), switchH * 0.5f);
        drawList->AddRect(switchMin, switchMax, ImColor(colors.Border), switchH * 0.5f, 0, 1.0f);

        // Thumb gliding
        const float thumbRadius = 6.5f * dpi;
        const float startThumbX = switchMin.x + thumbRadius + 2.5f * dpi;
        const float endThumbX = switchMax.x - thumbRadius - 2.5f * dpi;
        const float curThumbX = startThumbX + (endThumbX - startThumbX) * checkProgress;
        const ImVec2 thumbCenter = ImVec2(curThumbX, switchMin.y + switchH * 0.5f);

        // Thumb shadow + circle
        drawList->AddCircleFilled(ImVec2(thumbCenter.x, thumbCenter.y + 1.0f * dpi), thumbRadius, IM_COL32(0, 0, 0, 60));
        drawList->AddCircleFilled(thumbCenter, thumbRadius, options.disabled ? ImU32(ImColor(colors.TextMuted)) : IM_COL32(255, 255, 255, 255));
    } else {
        // Square Checkbox dimensions
        const float boxDim = 18.0f * dpi;
        const float boxY = bb.Min.y + (rowHeight - boxDim) * 0.5f;
        const ImVec2 boxMin = ImVec2(bb.Max.x - boxDim - 4.0f * dpi, boxY);
        const ImVec2 boxMax = ImVec2(boxMin.x + boxDim, boxMin.y + boxDim);

        ImVec4 boxBg = ImVec4(
            colors.CardBg.x + (colors.Accent.x - colors.CardBg.x) * checkProgress,
            colors.CardBg.y + (colors.Accent.y - colors.CardBg.y) * checkProgress,
            colors.CardBg.z + (colors.Accent.z - colors.CardBg.z) * checkProgress,
            options.disabled ? 0.35f : 1.0f
        );

        drawList->AddRectFilled(boxMin, boxMax, ImColor(boxBg), 4.0f * dpi);
        drawList->AddRect(boxMin, boxMax, ImColor(hovered ? colors.BorderStrong : colors.Border), 4.0f * dpi, 0, 1.0f);

        // Animated Checkmark
        if (checkProgress > 0.05f) {
            const ImVec2 p1 = ImVec2(boxMin.x + 4.5f * dpi, boxMin.y + 9.5f * dpi);
            const ImVec2 p2 = ImVec2(boxMin.x + 7.5f * dpi, boxMin.y + 13.0f * dpi);
            const ImVec2 p3 = ImVec2(boxMin.x + 13.5f * dpi, boxMin.y + 5.5f * dpi);

            // Interpolate check lines
            const ImVec2 curP2 = ImVec2(p1.x + (p2.x - p1.x) * std::min(checkProgress * 2.0f, 1.0f),
                                        p1.y + (p2.y - p1.y) * std::min(checkProgress * 2.0f, 1.0f));
            drawList->AddLine(p1, curP2, IM_COL32(255, 255, 255, static_cast<int>(255 * checkProgress)), 2.0f * dpi);

            if (checkProgress > 0.5f) {
                const float secondHalf = (checkProgress - 0.5f) * 2.0f;
                const ImVec2 curP3 = ImVec2(p2.x + (p3.x - p2.x) * secondHalf, p2.y + (p3.y - p2.y) * secondHalf);
                drawList->AddLine(p2, curP3, IM_COL32(255, 255, 255, static_cast<int>(255 * checkProgress)), 2.0f * dpi);
            }
        }
    }

    return pressed;
}

bool Checkbox(const char* label, bool* v, const char* description, const char* icon, bool disabled) {
    ToggleOptions opt;
    opt.description = description;
    opt.icon = icon;
    opt.disabled = disabled;
    opt.style = ToggleStyle::Checkbox;
    return ToggleEx(label, v, opt);
}

bool Toggle(const char* label, bool* v, const char* description, const char* icon, bool disabled) {
    ToggleOptions opt;
    opt.description = description;
    opt.icon = icon;
    opt.disabled = disabled;
    opt.style = ToggleStyle::PillSwitch;
    return ToggleEx(label, v, opt);
}

} // namespace parallax
