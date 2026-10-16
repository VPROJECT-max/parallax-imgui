#include "parallax/widgets/slider.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <cstdio>
#include <algorithm>
#include <cmath>

namespace parallax {

namespace {

void DrawFloatingValueCallout(const ImVec2& anchorTopCenter, const char* text, float dpi) {
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    const auto& colors = StyleManager::Get().Colors();

    PushFont(FontWeight::Caption);
    const ImVec2 textSz = ImGui::CalcTextSize(text);
    const float padX = 7.0f * dpi;
    const float padY = 3.5f * dpi;
    const float boxW = textSz.x + padX * 2.0f;
    const float boxH = textSz.y + padY * 2.0f;

    const ImVec2 boxMin(anchorTopCenter.x - boxW * 0.5f, anchorTopCenter.y - boxH - 6.0f * dpi);
    const ImVec2 boxMax(anchorTopCenter.x + boxW * 0.5f, anchorTopCenter.y - 6.0f * dpi);

    fg->AddRectFilled(
        ImVec2(boxMin.x, boxMin.y + 1.5f * dpi),
        ImVec2(boxMax.x, boxMax.y + 1.5f * dpi),
        ImColor(colors.Shadow),
        4.0f * dpi
    );
    fg->AddRectFilled(boxMin, boxMax, ImColor(colors.PopupBg), 4.0f * dpi);
    fg->AddRect(boxMin, boxMax, ImColor(colors.Accent), 4.0f * dpi, 0, 1.0f);
    fg->AddText(ImVec2(boxMin.x + padX, boxMin.y + padY), ImColor(colors.TextPrimary), text);
    PopFont();
}

} // namespace

bool SliderFloatEx(
    const char* label,
    float* v,
    float v_min,
    float v_max,
    const SliderOptions& options
) {
    if (!v) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);
    const float availW = (options.width > 0.0f) ? options.width : ImGui::GetContentRegionAvail().x;
    const char* fmt = (options.format && options.format[0]) ? options.format : "%.2f";

    // Direct numeric input state (Ctrl+Click or Double-Click)
    static ImGuiID s_activeDirectInputId = 0;
    if (s_activeDirectInputId == id) {
        ImGui::SetNextItemWidth(availW);
        bool edited = false;
        if (ImGui::InputFloat(label, v, 0.0f, 0.0f, fmt, ImGuiInputTextFlags_EnterReturnsTrue)) {
            *v = std::clamp(*v, v_min, v_max);
            edited = true;
            s_activeDirectInputId = 0;
        }
        if (!ImGui::IsItemActive() && !ImGui::IsMouseClicked(0)) {
            *v = std::clamp(*v, v_min, v_max);
            s_activeDirectInputId = 0;
        }
        return edited;
    }

    char valText[64];
    snprintf(valText, sizeof(valText), fmt, *v);
    PushFont(FontWeight::Caption);
    const ImVec2 valTextSize = ImGui::CalcTextSize(valText);
    PopFont();

    const float badgePadX = 6.0f * dpi;
    const float badgeW = valTextSize.x + badgePadX * 2.0f;
    const float badgeH = (options.compact ? 17.0f : 19.0f) * dpi;

    const float rowHeight = options.twoLine
        ? ((options.compact ? 36.0f : 42.0f) * dpi)
        : ((options.compact ? 24.0f : 30.0f) * dpi);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowHeight));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    if (ImGui::IsItemHovered() && !options.disabled) {
        if ((ImGui::GetIO().KeyCtrl && ImGui::IsMouseClicked(0)) || ImGui::IsMouseDoubleClicked(0)) {
            s_activeDirectInputId = id;
            return false;
        }
    }

    ImRect trackBb;
    ImRect badgeBb;
    const float trackH = (options.compact ? 5.5f : 7.0f) * dpi;

    if (options.twoLine) {
        const ImVec4 labelCol = options.disabled ? colors.TextMuted : colors.TextPrimary;
        PushFont(FontWeight::Medium);
        const char* textEnd = ImGui::FindRenderedTextEnd(label);
        drawList->AddText(nullptr, 0.0f, ImVec2(bb.Min.x, bb.Min.y + 1.0f * dpi), ImColor(labelCol), label, textEnd);
        PopFont();

        badgeBb = ImRect(
            ImVec2(bb.Max.x - badgeW, bb.Min.y),
            ImVec2(bb.Max.x, bb.Min.y + badgeH)
        );

        const float trackY = bb.Max.y - trackH - 5.0f * dpi;
        trackBb = ImRect(ImVec2(bb.Min.x + 4.0f * dpi, trackY), ImVec2(bb.Max.x - 4.0f * dpi, trackY + trackH));
    } else {
        PushFont(FontWeight::Medium);
        const char* textEnd = ImGui::FindRenderedTextEnd(label);
        const ImVec2 labelSize = ImGui::CalcTextSize(label, textEnd);
        const ImVec4 labelCol = options.disabled ? colors.TextMuted : colors.TextPrimary;
        drawList->AddText(
            nullptr,
            0.0f,
            ImVec2(bb.Min.x, bb.Min.y + (rowHeight - labelSize.y) * 0.5f),
            ImColor(labelCol),
            label,
            textEnd
        );
        PopFont();

        const float badgeY = bb.Min.y + (rowHeight - badgeH) * 0.5f;
        badgeBb = ImRect(ImVec2(bb.Max.x - badgeW, badgeY), ImVec2(bb.Max.x, badgeY + badgeH));

        const float trackW = std::max(availW - labelSize.x - badgeW - 26.0f * dpi, 84.0f * dpi);
        const float trackX = badgeBb.Min.x - trackW - 10.0f * dpi;
        const float trackY = bb.Min.y + (rowHeight - trackH) * 0.5f;
        trackBb = ImRect(ImVec2(trackX, trackY), ImVec2(trackX + trackW, trackY + trackH));
    }

    // Value badge pill
    const bool badgeHovered = badgeBb.Contains(ImGui::GetIO().MousePos) && !options.disabled;
    if (badgeHovered && ImGui::IsMouseClicked(0)) {
        s_activeDirectInputId = id;
        return false;
    }
    drawList->AddRectFilled(
        badgeBb.Min,
        badgeBb.Max,
        ImColor(badgeHovered ? colors.WidgetHover : colors.WidgetBg),
        4.0f * dpi
    );
    drawList->AddRect(
        badgeBb.Min,
        badgeBb.Max,
        ImColor(badgeHovered ? colors.Accent : colors.BorderSubtle),
        4.0f * dpi,
        0,
        1.0f
    );
    PushFont(FontWeight::Caption);
    drawList->AddText(
        ImVec2(badgeBb.Min.x + badgePadX, badgeBb.Min.y + (badgeH - valTextSize.y) * 0.5f),
        ImColor(options.disabled ? colors.TextMuted : colors.Accent),
        valText
    );
    PopFont();

    // Track interaction
    const ImRect hitBb(
        ImVec2(trackBb.Min.x - 6.0f * dpi, trackBb.Min.y - 8.0f * dpi),
        ImVec2(trackBb.Max.x + 6.0f * dpi, trackBb.Max.y + 8.0f * dpi)
    );

    bool hovered = false;
    bool held = false;
    bool changed = false;

    if (!options.disabled) {
        ImGui::ButtonBehavior(hitBb, id, &hovered, &held);

        if (held) {
            const float mouseX = ImGui::GetIO().MousePos.x;
            const float norm = std::clamp((mouseX - trackBb.Min.x) / std::max(1.0f, trackBb.GetWidth()), 0.0f, 1.0f);
            const float newVal = v_min + norm * (v_max - v_min);
            if (std::abs(*v - newVal) > 1e-5f) {
                *v = newVal;
                changed = true;
            }
        }
    }

    const float rawT = (v_max > v_min) ? std::clamp((*v - v_min) / (v_max - v_min), 0.0f, 1.0f) : 0.0f;
    const float t = ctx.Anim().SpringFloat(id, rawT, {260.0f, 25.0f, 1.0f});
    const float hoverAnim = ctx.Anim().AnimFloat(id + 0x3100, (hovered || held) ? 1.0f : 0.0f, metrics.AnimFast);

    const float trackRounding = trackH * 0.5f;

    // Recessed track surface
    drawList->AddRectFilled(trackBb.Min, trackBb.Max, ImColor(colors.WidgetBg), trackRounding);
    drawList->AddRect(
        trackBb.Min,
        trackBb.Max,
        ImColor(hovered ? colors.BorderStrong : colors.Border),
        trackRounding,
        0,
        1.0f
    );

    // Subtle internal reference tick marks
    if (options.tickCount > 1 && trackBb.GetWidth() > 60.0f * dpi) {
        const float midY = (trackBb.Min.y + trackBb.Max.y) * 0.5f;
        for (int i = 1; i < options.tickCount - 1; ++i) {
            const float tickRatio = static_cast<float>(i) / static_cast<float>(options.tickCount - 1);
            const float tickX = trackBb.Min.x + trackBb.GetWidth() * tickRatio;
            ImVec4 tickCol = (tickRatio <= t) ? colors.TextOnAccent : colors.TextMuted;
            tickCol.w *= 0.35f;
            drawList->AddCircleFilled(ImVec2(tickX, midY), 1.2f * dpi, ImColor(tickCol), 8);
        }
    }

    // Active filled track bar
    const float thumbX = trackBb.Min.x + trackBb.GetWidth() * t;
    if (t > 0.002f) {
        const ImVec4 fillCol = options.disabled ? colors.TextMuted : (held ? colors.AccentHover : colors.Accent);
        drawList->AddRectFilled(
            trackBb.Min,
            ImVec2(std::max(trackBb.Min.x + trackRounding, thumbX), trackBb.Max.y),
            ImColor(fillCol),
            trackRounding
        );
    }

    // Custom Parallax Precision Capsule Thumb
    const float thumbW = (options.compact ? 9.0f : 11.0f) * dpi + (held ? 2.0f * dpi : 0.0f);
    const float thumbH = (options.compact ? 14.0f : 17.0f) * dpi + (hoverAnim * 1.5f * dpi);
    const float midY = (trackBb.Min.y + trackBb.Max.y) * 0.5f;
    const ImVec2 thumbMin(thumbX - thumbW * 0.5f, midY - thumbH * 0.5f);
    const ImVec2 thumbMax(thumbX + thumbW * 0.5f, midY + thumbH * 0.5f);
    const float thumbRounding = 4.0f * dpi;

    // Hover/active halo
    if (hoverAnim > 0.01f && !options.disabled) {
        ImVec4 halo = colors.AccentMuted;
        halo.w *= (held ? 0.85f : 0.55f) * hoverAnim;
        drawList->AddRectFilled(
            ImVec2(thumbMin.x - 3.0f * dpi, thumbMin.y - 2.5f * dpi),
            ImVec2(thumbMax.x + 3.0f * dpi, thumbMax.y + 2.5f * dpi),
            ImColor(halo),
            thumbRounding + 2.5f * dpi
        );
    }

    // Thumb shadow + body + precision center grip notch
    if (!options.disabled) {
        drawList->AddRectFilled(
            ImVec2(thumbMin.x, thumbMin.y + 1.5f * dpi),
            ImVec2(thumbMax.x, thumbMax.y + 1.5f * dpi),
            IM_COL32(0, 0, 0, 70),
            thumbRounding
        );
    }

    const ImVec4 thumbBodyCol = options.disabled ? colors.WidgetHover : colors.ToggleThumbOn;
    const ImVec4 thumbBorderCol = options.disabled ? colors.Border : (held ? colors.AccentActive : colors.Accent);
    drawList->AddRectFilled(thumbMin, thumbMax, ImColor(thumbBodyCol), thumbRounding);
    drawList->AddRect(thumbMin, thumbMax, ImColor(thumbBorderCol), thumbRounding, 0, 1.5f);

    // Internal vertical notch inside the thumb
    drawList->AddLine(
        ImVec2(thumbX, midY - 3.0f * dpi),
        ImVec2(thumbX, midY + 3.0f * dpi),
        ImColor(thumbBorderCol),
        1.4f * dpi
    );

    if (held && options.floatingValue && !options.disabled) {
        snprintf(valText, sizeof(valText), fmt, *v);
        DrawFloatingValueCallout(ImVec2(thumbX, thumbMin.y), valText, dpi);
    }

    return changed;
}

bool SliderFloat(
    const char* label,
    float* v,
    float v_min,
    float v_max,
    const char* format,
    float width,
    bool twoLine,
    bool disabled
) {
    SliderOptions opt;
    opt.format = format ? format : "%.2f";
    opt.width = width;
    opt.twoLine = twoLine;
    opt.disabled = disabled;
    return SliderFloatEx(label, v, v_min, v_max, opt);
}

bool SliderInt(
    const char* label,
    int* v,
    int v_min,
    int v_max,
    const char* format,
    float width,
    bool twoLine,
    bool disabled
) {
    if (!v) return false;
    float fval = static_cast<float>(*v);
    char intValText[64];
    snprintf(intValText, sizeof(intValText), (format && format[0]) ? format : "%d", *v);

    // Use "%.0f" for internal float formatting if format uses %d
    const bool usesIntFmt = (!format || std::strstr(format, "d") != nullptr || std::strstr(format, "i") != nullptr);
    const char* floatFmt = usesIntFmt ? "%.0f" : format;

    SliderOptions opt;
    opt.format = floatFmt;
    opt.width = width;
    opt.twoLine = twoLine;
    opt.disabled = disabled;
    opt.tickCount = std::min(11, std::max(2, (v_max - v_min) + 1));

    if (SliderFloatEx(label, &fval, static_cast<float>(v_min), static_cast<float>(v_max), opt)) {
        *v = std::clamp(static_cast<int>(std::round(fval)), v_min, v_max);
        return true;
    }
    return false;
}

bool RangeSliderFloat(
    const char* label,
    float* v_min,
    float* v_max,
    float min_limit,
    float max_limit,
    const char* format,
    float width,
    bool disabled
) {
    if (!v_min || !v_max) return false;

    if (*v_min > *v_max) std::swap(*v_min, *v_max);
    *v_min = std::clamp(*v_min, min_limit, max_limit);
    *v_max = std::clamp(*v_max, min_limit, max_limit);

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);
    const float availW = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
    const char* fmt = (format && format[0]) ? format : "%.2f";

    // Direct numeric editing for Range Slider (Ctrl+Click or Double-Click or clicking range badge)
    static ImGuiID s_activeRangeEditId = 0;
    if (s_activeRangeEditId == id) {
        bool edited = false;
        const float halfW = (availW - 8.0f * dpi) * 0.5f;
        char minId[64], maxId[64];
        snprintf(minId, sizeof(minId), "##RangeMin_%08X", id);
        snprintf(maxId, sizeof(maxId), "##RangeMax_%08X", id);

        ImGui::SetNextItemWidth(halfW);
        if (ImGui::InputFloat(minId, v_min, 0.0f, 0.0f, fmt)) {
            *v_min = std::clamp(*v_min, min_limit, *v_max);
            edited = true;
        }
        bool minActive = ImGui::IsItemActive();

        ImGui::SameLine(0.0f, 8.0f * dpi);
        ImGui::SetNextItemWidth(halfW);
        if (ImGui::InputFloat(maxId, v_max, 0.0f, 0.0f, fmt)) {
            *v_max = std::clamp(*v_max, *v_min, max_limit);
            edited = true;
        }
        bool maxActive = ImGui::IsItemActive();

        if (!minActive && !maxActive && ImGui::IsMouseClicked(0)) {
            s_activeRangeEditId = 0;
        }
        return edited;
    }

    char fmtMin[32], fmtMax[32], rangeText[80];
    snprintf(fmtMin, sizeof(fmtMin), fmt, *v_min);
    snprintf(fmtMax, sizeof(fmtMax), fmt, *v_max);
    snprintf(rangeText, sizeof(rangeText), "%s  -  %s", fmtMin, fmtMax);

    PushFont(FontWeight::Caption);
    const ImVec2 rangeTextSize = ImGui::CalcTextSize(rangeText);
    PopFont();

    const float badgePadX = 7.0f * dpi;
    const float badgeW = rangeTextSize.x + badgePadX * 2.0f;
    const float badgeH = 19.0f * dpi;

    const float rowHeight = 42.0f * dpi;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowHeight));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    if (ImGui::IsItemHovered() && !disabled) {
        if ((ImGui::GetIO().KeyCtrl && ImGui::IsMouseClicked(0)) || ImGui::IsMouseDoubleClicked(0)) {
            s_activeRangeEditId = id;
            return false;
        }
    }

    // Header line: Label on left, recessed range pill on right
    PushFont(FontWeight::Medium);
    const char* textEnd = ImGui::FindRenderedTextEnd(label);
    drawList->AddText(
        nullptr,
        0.0f,
        ImVec2(bb.Min.x, bb.Min.y + 1.0f * dpi),
        ImColor(disabled ? colors.TextMuted : colors.TextPrimary),
        label,
        textEnd
    );
    PopFont();

    const ImRect badgeBb(ImVec2(bb.Max.x - badgeW, bb.Min.y), ImVec2(bb.Max.x, bb.Min.y + badgeH));
    const bool badgeHovered = badgeBb.Contains(ImGui::GetIO().MousePos) && !disabled;
    if (badgeHovered && ImGui::IsMouseClicked(0)) {
        s_activeRangeEditId = id;
        return false;
    }

    drawList->AddRectFilled(
        badgeBb.Min,
        badgeBb.Max,
        ImColor(badgeHovered ? colors.WidgetHover : colors.WidgetBg),
        4.0f * dpi
    );
    drawList->AddRect(
        badgeBb.Min,
        badgeBb.Max,
        ImColor(badgeHovered ? colors.Accent : colors.BorderSubtle),
        4.0f * dpi,
        0,
        1.0f
    );
    PushFont(FontWeight::Caption);
    drawList->AddText(
        ImVec2(badgeBb.Min.x + badgePadX, badgeBb.Min.y + (badgeH - rangeTextSize.y) * 0.5f),
        ImColor(disabled ? colors.TextMuted : colors.Accent),
        rangeText
    );
    PopFont();

    const float trackH = 7.0f * dpi;
    const float trackY = bb.Max.y - trackH - 5.0f * dpi;
    const ImRect trackBb(ImVec2(bb.Min.x + 5.0f * dpi, trackY), ImVec2(bb.Max.x - 5.0f * dpi, trackY + trackH));

    const float totalRange = max_limit - min_limit;
    float t1 = (totalRange > 0.0001f) ? std::clamp((*v_min - min_limit) / totalRange, 0.0f, 1.0f) : 0.0f;
    float t2 = (totalRange > 0.0001f) ? std::clamp((*v_max - min_limit) / totalRange, 0.0f, 1.0f) : 1.0f;

    float thumb1X = trackBb.Min.x + trackBb.GetWidth() * t1;
    float thumb2X = trackBb.Min.x + trackBb.GetWidth() * t2;

    bool changed = false;

    // Active thumb tracking: 1 = min thumb, 2 = max thumb, 0 = none, 3 = pending overlap disambiguation
    static int s_activeRangeThumb = 0;
    static ImGuiID s_activeRangeId = 0;
    static float s_clickStartX = 0.0f;

    const ImRect hitBb(
        ImVec2(trackBb.Min.x - 8.0f * dpi, trackBb.Min.y - 10.0f * dpi),
        ImVec2(trackBb.Max.x + 8.0f * dpi, trackBb.Max.y + 10.0f * dpi)
    );

    bool hovered = false;
    bool held = false;

    if (!disabled) {
        ImGui::ButtonBehavior(hitBb, id, &hovered, &held);

        if (ImGui::IsMouseClicked(0) && hovered) {
            s_activeRangeId = id;
            const float mx = ImGui::GetIO().MousePos.x;
            s_clickStartX = mx;
            const float dist1 = std::abs(mx - thumb1X);
            const float dist2 = std::abs(mx - thumb2X);
            if (std::abs(thumb2X - thumb1X) < 4.0f * dpi) {
                // Thumbs are overlapping: disambiguate by click side or drag direction
                if (mx < thumb1X - 1.5f * dpi) {
                    s_activeRangeThumb = 1;
                } else if (mx > thumb2X + 1.5f * dpi) {
                    s_activeRangeThumb = 2;
                } else {
                    s_activeRangeThumb = 3; // Decide on first drag movement
                }
            } else {
                s_activeRangeThumb = (dist1 <= dist2) ? 1 : 2;
            }
        }

        if (held && s_activeRangeId == id) {
            const float mouseX = ImGui::GetIO().MousePos.x;
            if (s_activeRangeThumb == 3 && std::abs(mouseX - s_clickStartX) > 1.5f * dpi) {
                s_activeRangeThumb = (mouseX < s_clickStartX) ? 1 : 2;
            }

            const float newT = std::clamp((mouseX - trackBb.Min.x) / std::max(1.0f, trackBb.GetWidth()), 0.0f, 1.0f);
            const float newVal = min_limit + newT * totalRange;

            if (s_activeRangeThumb == 1) {
                const float clamped = std::clamp(newVal, min_limit, *v_max);
                if (std::abs(*v_min - clamped) > 1e-5f) {
                    *v_min = clamped;
                    changed = true;
                }
            } else if (s_activeRangeThumb == 2) {
                const float clamped = std::clamp(newVal, *v_min, max_limit);
                if (std::abs(*v_max - clamped) > 1e-5f) {
                    *v_max = clamped;
                    changed = true;
                }
            }
        } else if (!held && s_activeRangeId == id) {
            s_activeRangeId = 0;
            s_activeRangeThumb = 0;
        }
    }

    // Spring-smoothed thumb positions
    t1 = ctx.Anim().SpringFloat(
        id + 1,
        (totalRange > 0.0001f) ? std::clamp((*v_min - min_limit) / totalRange, 0.0f, 1.0f) : 0.0f,
        {260.0f, 25.0f, 1.0f}
    );
    t2 = ctx.Anim().SpringFloat(
        id + 2,
        (totalRange > 0.0001f) ? std::clamp((*v_max - min_limit) / totalRange, 0.0f, 1.0f) : 1.0f,
        {260.0f, 25.0f, 1.0f}
    );
    thumb1X = trackBb.Min.x + trackBb.GetWidth() * t1;
    thumb2X = trackBb.Min.x + trackBb.GetWidth() * t2;

    const float trackRounding = trackH * 0.5f;

    // Recessed track background
    drawList->AddRectFilled(trackBb.Min, trackBb.Max, ImColor(colors.WidgetBg), trackRounding);
    drawList->AddRect(
        trackBb.Min,
        trackBb.Max,
        ImColor(hovered ? colors.BorderStrong : colors.Border),
        trackRounding,
        0,
        1.0f
    );

    // Reference ticks along track
    const float midY = (trackBb.Min.y + trackBb.Max.y) * 0.5f;
    for (int i = 1; i < 4; ++i) {
        const float ratio = static_cast<float>(i) * 0.25f;
        const float tx = trackBb.Min.x + trackBb.GetWidth() * ratio;
        ImVec4 tickCol = colors.TextMuted;
        tickCol.w *= 0.30f;
        drawList->AddCircleFilled(ImVec2(tx, midY), 1.2f * dpi, ImColor(tickCol), 8);
    }

    // Highlighted selected range span
    if (thumb2X > thumb1X + 1.0f) {
        const ImVec4 spanCol = disabled ? colors.TextMuted : colors.Accent;
        drawList->AddRectFilled(
            ImVec2(thumb1X, trackBb.Min.y),
            ImVec2(thumb2X, trackBb.Max.y),
            ImColor(spanCol),
            trackRounding
        );
    }

    // Two distinguishable sculpted capsule handles (Min = left bracket cue, Max = right bracket cue)
    auto drawRangeHandle = [&](float cx, bool isMin, bool isActive) {
        const float hw = (isActive ? 6.0f : 5.0f) * dpi;
        const float hh = (isActive ? 9.0f : 8.0f) * dpi;
        const ImVec2 hMin(cx - hw, midY - hh);
        const ImVec2 hMax(cx + hw, midY + hh);
        const float hRound = 3.5f * dpi;

        if (isActive && !disabled) {
            ImVec4 halo = colors.AccentMuted;
            halo.w = 0.75f;
            drawList->AddRectFilled(
                ImVec2(hMin.x - 3.0f * dpi, hMin.y - 2.5f * dpi),
                ImVec2(hMax.x + 3.0f * dpi, hMax.y + 2.5f * dpi),
                ImColor(halo),
                hRound + 2.5f * dpi
            );
        }

        if (!disabled) {
            drawList->AddRectFilled(
                ImVec2(hMin.x, hMin.y + 1.5f * dpi),
                ImVec2(hMax.x, hMax.y + 1.5f * dpi),
                IM_COL32(0, 0, 0, 70),
                hRound
            );
        }

        const ImVec4 bodyCol = disabled ? colors.WidgetHover : colors.ToggleThumbOn;
        const ImVec4 rimCol = disabled ? colors.Border : (isActive ? colors.AccentActive : colors.Accent);
        drawList->AddRectFilled(hMin, hMax, ImColor(bodyCol), hRound);
        drawList->AddRect(hMin, hMax, ImColor(rimCol), hRound, 0, 1.5f);

        // Directional micro-chevron inside handle so Min and Max are visually distinct
        const float dir = isMin ? -1.0f : 1.0f;
        drawList->AddLine(
            ImVec2(cx - dir * 1.2f * dpi, midY - 2.5f * dpi),
            ImVec2(cx + dir * 1.2f * dpi, midY),
            ImColor(rimCol),
            1.3f * dpi
        );
        drawList->AddLine(
            ImVec2(cx + dir * 1.2f * dpi, midY),
            ImVec2(cx - dir * 1.2f * dpi, midY + 2.5f * dpi),
            ImColor(rimCol),
            1.3f * dpi
        );
    };

    const bool minActive = (held && s_activeRangeId == id && s_activeRangeThumb == 1);
    const bool maxActive = (held && s_activeRangeId == id && s_activeRangeThumb == 2);

    drawRangeHandle(thumb1X, true, minActive);
    drawRangeHandle(thumb2X, false, maxActive);

    if (minActive && !disabled) {
        snprintf(fmtMin, sizeof(fmtMin), fmt, *v_min);
        DrawFloatingValueCallout(ImVec2(thumb1X, midY - 9.0f * dpi), fmtMin, dpi);
    } else if (maxActive && !disabled) {
        snprintf(fmtMax, sizeof(fmtMax), fmt, *v_max);
        DrawFloatingValueCallout(ImVec2(thumb2X, midY - 9.0f * dpi), fmtMax, dpi);
    }

    return changed;
}

bool RangeSliderInt(
    const char* label,
    int* v_min,
    int* v_max,
    int min_limit,
    int max_limit,
    const char* format,
    float width,
    bool disabled
) {
    if (!v_min || !v_max) return false;
    float fmin = static_cast<float>(*v_min);
    float fmax = static_cast<float>(*v_max);
    const bool usesIntFmt = (!format || std::strstr(format, "d") != nullptr || std::strstr(format, "i") != nullptr);
    const char* floatFmt = usesIntFmt ? "%.0f" : format;

    if (RangeSliderFloat(label, &fmin, &fmax, static_cast<float>(min_limit), static_cast<float>(max_limit), floatFmt, width, disabled)) {
        *v_min = std::clamp(static_cast<int>(std::round(fmin)), min_limit, max_limit);
        *v_max = std::clamp(static_cast<int>(std::round(fmax)), min_limit, max_limit);
        return true;
    }
    return false;
}

} // namespace parallax
