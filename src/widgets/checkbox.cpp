#include "parallax/widgets/checkbox.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>

namespace parallax {

namespace {

constexpr ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t) noexcept {
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    );
}

} // namespace

void DrawParallaxSwitchGlyph(
    ImDrawList* drawList,
    const ImVec2& switchMin,
    const ImVec2& switchMax,
    float checkProgress,
    float hoverProgress,
    bool held,
    bool disabled
) {
    const float dpi = Context::Get().GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();

    const float switchW = switchMax.x - switchMin.x;
    const float switchH = switchMax.y - switchMin.y;
    const float rounding = switchH * 0.46f;
    const float t = std::clamp(checkProgress, 0.0f, 1.0f);

    // Subtle outer hover/focus halo
    if (hoverProgress > 0.01f && !disabled) {
        const float expand = 2.5f * dpi * hoverProgress;
        ImVec4 haloCol = colors.AccentMuted;
        haloCol.w *= 0.65f * hoverProgress;
        drawList->AddRectFilled(
            ImVec2(switchMin.x - expand, switchMin.y - expand),
            ImVec2(switchMax.x + expand, switchMax.y + expand),
            ImColor(haloCol),
            rounding + expand
        );
    }

    // Track surface: interpolates from WidgetBg/WidgetHover to Accent
    const ImVec4 offTrack = LerpColor(colors.WidgetBg, colors.WidgetHover, hoverProgress * 0.65f);
    ImVec4 trackCol = LerpColor(offTrack, colors.Accent, t);
    if (disabled) {
        trackCol.w *= 0.45f;
    }

    drawList->AddRectFilled(switchMin, switchMax, ImColor(trackCol), rounding);

    // Track border: crisp structural edge when off, subtle inner rim when on
    ImVec4 borderCol = LerpColor(
        LerpColor(colors.Border, colors.BorderStrong, hoverProgress),
        colors.AccentActive,
        t
    );
    if (disabled) {
        borderCol.w *= 0.45f;
    }
    drawList->AddRect(switchMin, switchMax, ImColor(borderCol), rounding, 0, 1.0f);

    // Micro state markers inside the track: subtle 'I' bar on left (active), tiny ring/dot on right (inactive)
    if (!disabled && switchW >= 30.0f * dpi) {
        const float centerY = switchMin.y + switchH * 0.5f;
        if (t > 0.25f) {
            const float markerAlpha = std::clamp((t - 0.25f) / 0.75f, 0.0f, 1.0f) * 0.55f;
            ImVec4 onMarker = colors.TextOnAccent;
            onMarker.w = markerAlpha;
            const float barX = switchMin.x + switchW * 0.23f;
            drawList->AddLine(
                ImVec2(barX, centerY - 3.0f * dpi),
                ImVec2(barX, centerY + 3.0f * dpi),
                ImColor(onMarker),
                1.4f * dpi
            );
        }
        if (t < 0.75f) {
            const float markerAlpha = std::clamp((0.75f - t) / 0.75f, 0.0f, 1.0f) * 0.45f;
            ImVec4 offMarker = colors.TextMuted;
            offMarker.w = markerAlpha;
            const float dotX = switchMax.x - switchW * 0.23f;
            drawList->AddCircle(ImVec2(dotX, centerY), 2.0f * dpi, ImColor(offMarker), 12, 1.2f * dpi);
        }
    }

    // Tactile thumb with kinetic stretch during motion or hold
    const float pad = 2.5f * dpi;
    const float baseThumbH = switchH - pad * 2.0f;
    const float midFlight = 4.0f * t * (1.0f - t); // Peaks at t = 0.5
    const float stretch = (held ? 2.8f * dpi : 0.0f) + (midFlight * 2.2f * dpi);
    const float thumbW = baseThumbH + stretch;
    const float thumbRounding = baseThumbH * 0.44f;

    const float minThumbX = switchMin.x + pad;
    const float maxThumbX = switchMax.x - pad - thumbW;
    const float thumbX = minThumbX + (maxThumbX - minThumbX) * t;
    const ImVec2 thumbMin(thumbX, switchMin.y + pad);
    const ImVec2 thumbMax(thumbX + thumbW, switchMax.y - pad);

    // Thumb elevation shadow
    if (!disabled) {
        drawList->AddRectFilled(
            ImVec2(thumbMin.x, thumbMin.y + 1.0f * dpi),
            ImVec2(thumbMax.x, thumbMax.y + 1.2f * dpi),
            IM_COL32(0, 0, 0, 55),
            thumbRounding
        );
    }

    ImVec4 thumbCol = LerpColor(colors.ToggleThumbOff, colors.ToggleThumbOn, t);
    if (disabled) {
        thumbCol = colors.TextMuted;
        thumbCol.w = 0.55f;
    }
    drawList->AddRectFilled(thumbMin, thumbMax, ImColor(thumbCol), thumbRounding);

    // Signature Parallax micro-core indicator inside the thumb
    const ImVec2 thumbCenter((thumbMin.x + thumbMax.x) * 0.5f, (thumbMin.y + thumbMax.y) * 0.5f);
    ImVec4 coreCol = LerpColor(colors.WidgetBg, colors.Accent, t);
    coreCol.w = disabled ? 0.3f : (0.55f + 0.35f * t);
    drawList->AddCircleFilled(thumbCenter, 1.6f * dpi, ImColor(coreCol), 10);
}

void DrawParallaxCheckboxGlyph(
    ImDrawList* drawList,
    const ImVec2& boxMin,
    const ImVec2& boxMax,
    float checkProgress,
    float hoverProgress,
    bool indeterminate,
    bool disabled
) {
    const float dpi = Context::Get().GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();

    const float boxDim = boxMax.x - boxMin.x;
    const float rounding = std::max(4.5f * dpi, boxDim * 0.26f);
    const float activeAmount = indeterminate ? 1.0f : std::clamp(checkProgress, 0.0f, 1.0f);

    // Outer hover glow
    if (hoverProgress > 0.01f && !disabled) {
        const float expand = 2.2f * dpi * hoverProgress;
        ImVec4 haloCol = colors.AccentMuted;
        haloCol.w *= 0.60f * hoverProgress;
        drawList->AddRectFilled(
            ImVec2(boxMin.x - expand, boxMin.y - expand),
            ImVec2(boxMax.x + expand, boxMax.y + expand),
            ImColor(haloCol),
            rounding + expand
        );
    }

    // Recessed control base surface
    ImVec4 baseBg = LerpColor(colors.WidgetBg, colors.WidgetHover, hoverProgress);
    if (disabled) {
        baseBg.w *= 0.45f;
    }
    drawList->AddRectFilled(boxMin, boxMax, ImColor(baseBg), rounding);

    // Animated center-expanding accent fill when checked or indeterminate
    if (activeAmount > 0.01f) {
        const ImVec2 center((boxMin.x + boxMax.x) * 0.5f, (boxMin.y + boxMax.y) * 0.5f);
        const float halfExtent = (boxDim * 0.5f) * std::min(1.0f, activeAmount * 1.08f);
        ImVec4 fillCol = LerpColor(colors.Accent, colors.AccentHover, hoverProgress * 0.4f);
        if (disabled) {
            fillCol.w *= 0.45f;
        }
        drawList->AddRectFilled(
            ImVec2(center.x - halfExtent, center.y - halfExtent),
            ImVec2(center.x + halfExtent, center.y + halfExtent),
            ImColor(fillCol),
            rounding * activeAmount
        );
    }

    // Crisp border
    ImVec4 borderCol = LerpColor(
        LerpColor(colors.Border, colors.BorderStrong, hoverProgress),
        colors.AccentActive,
        activeAmount
    );
    if (disabled) {
        borderCol.w *= 0.45f;
    }
    drawList->AddRect(boxMin, boxMax, ImColor(borderCol), rounding, 0, 1.0f);

    // Glyph: Indeterminate horizontal bar OR two-segment animated checkmark
    ImVec4 glyphCol = colors.TextOnAccent;
    if (disabled) {
        glyphCol.w *= 0.55f;
    }

    if (indeterminate) {
        const float cy = (boxMin.y + boxMax.y) * 0.5f;
        const float padX = boxDim * 0.26f;
        drawList->AddLine(
            ImVec2(boxMin.x + padX, cy),
            ImVec2(boxMax.x - padX, cy),
            ImColor(glyphCol),
            2.0f * dpi
        );
    } else if (checkProgress > 0.05f) {
        const float t = std::clamp(checkProgress, 0.0f, 1.0f);
        glyphCol.w *= t;
        const ImU32 checkU32 = ImColor(glyphCol);

        const ImVec2 p1(boxMin.x + boxDim * 0.25f, boxMin.y + boxDim * 0.52f);
        const ImVec2 p2(boxMin.x + boxDim * 0.43f, boxMin.y + boxDim * 0.71f);
        const ImVec2 p3(boxMin.x + boxDim * 0.76f, boxMin.y + boxDim * 0.31f);

        const float seg1 = std::min(t * 2.0f, 1.0f);
        const ImVec2 curP2(p1.x + (p2.x - p1.x) * seg1, p1.y + (p2.y - p1.y) * seg1);
        drawList->AddLine(p1, curP2, checkU32, 2.0f * dpi);

        if (t > 0.5f) {
            const float seg2 = (t - 0.5f) * 2.0f;
            const ImVec2 curP3(p2.x + (p3.x - p2.x) * seg2, p2.y + (p3.y - p2.y) * seg2);
            drawList->AddLine(p2, curP3, checkU32, 2.0f * dpi);
        }
    }
}

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
    const float baseRowH = options.compact ? metrics.ControlHeightCompact : metrics.RowHeight;
    const float rowHeight = (hasDesc ? 40.0f : baseRowH) * dpi;

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

    const float targetCheck = *v ? 1.0f : 0.0f;
    const float checkProgress = ctx.Anim().SpringFloat(id, targetCheck, {220.0f, 22.0f, 1.0f});
    const float hoverProgress = ctx.Anim().AnimFloat(id + 0x10000, (hovered && !options.disabled) ? 1.0f : 0.0f, metrics.AnimFast);

    // Subtle row hover surface
    if (hoverProgress > 0.01f && !options.disabled) {
        ImVec4 rowHover = colors.CardHover;
        rowHover.w *= hoverProgress * 0.75f;
        drawList->AddRectFilled(
            ImVec2(bb.Min.x - 4.0f * dpi, bb.Min.y - 1.0f * dpi),
            ImVec2(bb.Max.x + 4.0f * dpi, bb.Max.y + 1.0f * dpi),
            ImColor(rowHover),
            metrics.FrameRounding
        );
    }

    // Left side: Icon + Label + Description
    float textCursorX = bb.Min.x + 4.0f * dpi;

    if (options.icon && options.icon[0] != '\0') {
        const float iconDim = (options.compact ? 14.0f : 16.0f) * dpi;
        const ImVec2 iconPos(textCursorX, bb.Min.y + (hasDesc ? 6.0f * dpi : (rowHeight - iconDim) * 0.5f));
        const ImVec4 iconCol = options.disabled
            ? colors.TextMuted
            : ((*v || hovered) ? colors.Accent : colors.TextSecondary);
        ctx.Icons().DrawIcon(drawList, options.icon, iconPos, ImVec2(iconDim, iconDim), iconCol);
        textCursorX += iconDim + 8.0f * dpi;
    }

    const ImVec4 labelCol = options.disabled ? colors.TextMuted : colors.TextPrimary;
    PushFont(FontWeight::Medium);
    const float labelY = hasDesc
        ? (bb.Min.y + 4.0f * dpi)
        : (bb.Min.y + (rowHeight - ImGui::GetFontSize()) * 0.5f);
    const char* textEnd = ImGui::FindRenderedTextEnd(label);
    drawList->AddText(nullptr, 0.0f, ImVec2(textCursorX, labelY), ImColor(labelCol), label, textEnd);
    PopFont();

    if (hasDesc) {
        const float descY = labelY + ImGui::GetFontSize() + 2.0f * dpi;
        PushFont(FontWeight::Caption);
        drawList->AddText(ImVec2(textCursorX, descY), ImColor(colors.TextSecondary), options.description);
        PopFont();
    }

    // Right side: Custom Parallax Switch or Checkbox
    if (options.style == ToggleStyle::PillSwitch) {
        const float switchW = (options.compact ? 30.0f : 36.0f) * dpi;
        const float switchH = (options.compact ? 16.0f : 19.0f) * dpi;
        const float switchY = bb.Min.y + (rowHeight - switchH) * 0.5f;
        const ImVec2 switchMin(bb.Max.x - switchW - 4.0f * dpi, switchY);
        const ImVec2 switchMax(switchMin.x + switchW, switchMin.y + switchH);

        DrawParallaxSwitchGlyph(drawList, switchMin, switchMax, checkProgress, hoverProgress, held, options.disabled);
    } else {
        const float boxDim = (options.compact ? 16.0f : 18.0f) * dpi;
        const float boxY = bb.Min.y + (rowHeight - boxDim) * 0.5f;
        const ImVec2 boxMin(bb.Max.x - boxDim - 4.0f * dpi, boxY);
        const ImVec2 boxMax(boxMin.x + boxDim, boxMin.y + boxDim);

        DrawParallaxCheckboxGlyph(drawList, boxMin, boxMax, checkProgress, hoverProgress, options.indeterminate, options.disabled);
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

bool CheckboxIndeterminate(const char* label, bool* v, bool indeterminate, const char* description, const char* icon, bool disabled) {
    ToggleOptions opt;
    opt.description = description;
    opt.icon = icon;
    opt.disabled = disabled;
    opt.indeterminate = indeterminate;
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

bool ToggleCompact(const char* label, bool* v, const char* description, const char* icon, bool disabled) {
    ToggleOptions opt;
    opt.description = description;
    opt.icon = icon;
    opt.disabled = disabled;
    opt.compact = true;
    opt.style = ToggleStyle::PillSwitch;
    return ToggleEx(label, v, opt);
}

} // namespace parallax
