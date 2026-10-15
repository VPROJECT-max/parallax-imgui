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
    if (!v) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);
    const float availW = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;

    // Track direct input text state (Ctrl+Click or Double-Click)
    static ImGuiID s_activeDirectInputId = 0;
    if (s_activeDirectInputId == id) {
        ImGui::SetNextItemWidth(availW);
        if (ImGui::InputFloat(label, v, 0.0f, 0.0f, format)) {
            *v = std::clamp(*v, v_min, v_max);
        }
        if (!ImGui::IsItemActive()) {
            s_activeDirectInputId = 0;
        }
        return true;
    }

    char valText[64];
    snprintf(valText, sizeof(valText), format ? format : "%.2f", *v);
    const ImVec2 valTextSize = ImGui::CalcTextSize(valText);

    const float rowHeight = twoLine ? (40.0f * dpi) : (28.0f * dpi);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowHeight));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    // Direct input trigger via Ctrl+Click or Double-Click
    if (ImGui::IsItemHovered() && !disabled) {
        if ((ImGui::GetIO().KeyCtrl && ImGui::IsMouseClicked(0)) || ImGui::IsMouseDoubleClicked(0)) {
            s_activeDirectInputId = id;
            return false;
        }
    }

    ImRect trackBb;
    if (twoLine) {
        // Label on top left, value badge on top right
        ImVec4 labelCol = disabled ? colors.TextMuted : colors.TextPrimary;
        PushFont(FontWeight::Medium);
        const char* textEnd = ImGui::FindRenderedTextEnd(label);
        drawList->AddText(nullptr, 0.0f, bb.Min, ImColor(labelCol), label, textEnd);
        PopFont();

        PushFont(FontWeight::Caption);
        drawList->AddText(ImVec2(bb.Max.x - valTextSize.x, bb.Min.y + 1.0f * dpi), ImColor(colors.Accent), valText);
        PopFont();

        const float trackH = 6.0f * dpi;
        const float trackY = bb.Max.y - trackH - 4.0f * dpi;
        trackBb = ImRect(ImVec2(bb.Min.x, trackY), ImVec2(bb.Max.x, trackY + trackH));
    } else {
        // Inline: label on left, slider + value on right
        PushFont(FontWeight::Medium);
        const char* textEnd = ImGui::FindRenderedTextEnd(label);
        const ImVec2 labelSize = ImGui::CalcTextSize(label, textEnd);
        ImVec4 labelCol = disabled ? colors.TextMuted : colors.TextPrimary;
        drawList->AddText(nullptr, 0.0f, ImVec2(bb.Min.x, bb.Min.y + (rowHeight - labelSize.y) * 0.5f), ImColor(labelCol), label, textEnd);
        PopFont();

        const float valWidth = valTextSize.x + 8.0f * dpi;
        const float trackW = std::max(availW - labelSize.x - valWidth - 28.0f * dpi, 100.0f * dpi);
        const float trackH = 6.0f * dpi;
        const float trackX = bb.Max.x - trackW - valWidth - 8.0f * dpi;
        const float trackY = bb.Min.y + (rowHeight - trackH) * 0.5f;

        trackBb = ImRect(ImVec2(trackX, trackY), ImVec2(trackX + trackW, trackY + trackH));

        // Value text badge on right
        PushFont(FontWeight::Caption);
        drawList->AddText(ImVec2(bb.Max.x - valTextSize.x, bb.Min.y + (rowHeight - labelSize.y) * 0.5f), ImColor(colors.Accent), valText);
        PopFont();
    }

    // Interaction handling on track
    const ImRect hitBb(ImVec2(trackBb.Min.x - 6.0f * dpi, trackBb.Min.y - 8.0f * dpi),
                       ImVec2(trackBb.Max.x + 6.0f * dpi, trackBb.Max.y + 8.0f * dpi));

    bool hovered = false;
    bool held = false;
    bool changed = false;

    if (!disabled) {
        ImGui::ButtonBehavior(hitBb, id, &hovered, &held);

        if (held) {
            float mouseX = ImGui::GetIO().MousePos.x;
            float t = std::clamp((mouseX - trackBb.Min.x) / trackBb.GetWidth(), 0.0f, 1.0f);
            float newVal = v_min + t * (v_max - v_min);
            if (std::abs(*v - newVal) > 1e-4f) {
                *v = newVal;
                changed = true;
            }

            // Floating value tooltip while dragging
            ImGui::SetTooltip(format ? format : "%.2f", *v);
        }
    }

    // Normalized progress (0 to 1)
    const float rawT = (v_max > v_min) ? std::clamp((*v - v_min) / (v_max - v_min), 0.0f, 1.0f) : 0.0f;
    const float t = ctx.Anim().SpringFloat(id, rawT, {240.0f, 24.0f, 1.0f});

    // Render recessed background track
    drawList->AddRectFilled(trackBb.Min, trackBb.Max, ImColor(colors.CardHover), 3.0f * dpi);
    drawList->AddRect(trackBb.Min, trackBb.Max, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);

    // Render active gradient filled track
    const float thumbX = trackBb.Min.x + trackBb.GetWidth() * t;
    if (t > 0.001f) {
        ImVec4 fillCol = disabled ? colors.TextMuted : colors.Accent;
        drawList->AddRectFilled(trackBb.Min, ImVec2(thumbX, trackBb.Max.y), ImColor(fillCol), 3.0f * dpi);
    }

    // Animated tactile thumb radius with physical spring
    const float targetRadius = (held ? 8.5f : (hovered ? 7.5f : 6.0f)) * dpi;
    const float thumbRadius = ctx.Anim().SpringFloat(id + 1, targetRadius, {260.0f, 22.0f, 1.0f});
    const ImVec2 thumbCenter = ImVec2(thumbX, trackBb.Min.y + trackBb.GetHeight() * 0.5f);

    // Ambient glow halo when held or hovered
    if ((held || hovered) && !disabled) {
        drawList->AddCircleFilled(thumbCenter, thumbRadius + 5.0f * dpi, ImColor(colors.Accent.x, colors.Accent.y, colors.Accent.z, held ? 0.35f : 0.20f));
    }

    // Drop shadow + thumb core
    drawList->AddCircleFilled(ImVec2(thumbCenter.x, thumbCenter.y + 1.5f * dpi), thumbRadius, IM_COL32(0, 0, 0, 90));
    drawList->AddCircleFilled(thumbCenter, thumbRadius, disabled ? ImU32(ImColor(colors.CardHover)) : IM_COL32(255, 255, 255, 255));
    drawList->AddCircle(thumbCenter, thumbRadius, ImColor(disabled ? colors.Border : colors.Accent), 20, 1.5f);

    return changed;
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
    if (SliderFloat(label, &fval, static_cast<float>(v_min), static_cast<float>(v_max), format ? format : "%d", width, twoLine, disabled)) {
        *v = static_cast<int>(std::round(fval));
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

    // Enforce invariant
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

    char fmtMin[32], fmtMax[32], rangeText[80];
    snprintf(fmtMin, sizeof(fmtMin), format ? format : "%.2f", *v_min);
    snprintf(fmtMax, sizeof(fmtMax), format ? format : "%.2f", *v_max);
    snprintf(rangeText, sizeof(rangeText), "%s - %s", fmtMin, fmtMax);
    const ImVec2 rangeTextSize = ImGui::CalcTextSize(rangeText);

    const float rowHeight = 40.0f * dpi;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowHeight));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    // Header line: Label on left, formatted min-max on right
    PushFont(FontWeight::Medium);
    const char* textEnd = ImGui::FindRenderedTextEnd(label);
    drawList->AddText(nullptr, 0.0f, bb.Min, ImColor(disabled ? colors.TextMuted : colors.TextPrimary), label, textEnd);
    PopFont();

    PushFont(FontWeight::Caption);
    drawList->AddText(ImVec2(bb.Max.x - rangeTextSize.x, bb.Min.y + 1.0f * dpi), ImColor(colors.Accent), rangeText);
    PopFont();

    const float trackH = 6.0f * dpi;
    const float trackY = bb.Max.y - trackH - 4.0f * dpi;
    const ImRect trackBb(ImVec2(bb.Min.x, trackY), ImVec2(bb.Max.x, trackY + trackH));

    const float totalRange = max_limit - min_limit;
    float t1 = (totalRange > 0.001f) ? std::clamp((*v_min - min_limit) / totalRange, 0.0f, 1.0f) : 0.0f;
    float t2 = (totalRange > 0.001f) ? std::clamp((*v_max - min_limit) / totalRange, 0.0f, 1.0f) : 1.0f;

    float thumb1X = trackBb.Min.x + trackBb.GetWidth() * t1;
    float thumb2X = trackBb.Min.x + trackBb.GetWidth() * t2;

    bool changed = false;

    // Active thumb tracking: 1 = min thumb, 2 = max thumb, 0 = none
    static int s_activeRangeThumb = 0;
    static ImGuiID s_activeRangeId = 0;

    const ImRect hitBb(ImVec2(trackBb.Min.x - 8.0f * dpi, trackBb.Min.y - 10.0f * dpi),
                       ImVec2(trackBb.Max.x + 8.0f * dpi, trackBb.Max.y + 10.0f * dpi));

    bool hovered = false;
    bool held = false;

    if (!disabled) {
        ImGui::ButtonBehavior(hitBb, id, &hovered, &held);

        if (ImGui::IsMouseClicked(0) && hovered) {
            s_activeRangeId = id;
            float mx = ImGui::GetIO().MousePos.x;
            float dist1 = std::abs(mx - thumb1X);
            float dist2 = std::abs(mx - thumb2X);
            if (std::abs(dist1 - dist2) < 6.0f * dpi) {
                s_activeRangeThumb = (mx < thumb1X) ? 1 : 2;
            } else {
                s_activeRangeThumb = (dist1 < dist2) ? 1 : 2;
            }
        }

        if (held && s_activeRangeId == id) {
            float mouseX = ImGui::GetIO().MousePos.x;
            float newT = std::clamp((mouseX - trackBb.Min.x) / trackBb.GetWidth(), 0.0f, 1.0f);
            float newVal = min_limit + newT * totalRange;

            if (s_activeRangeThumb == 1) {
                *v_min = std::clamp(newVal, min_limit, *v_max);
                changed = true;
                ImGui::SetTooltip("Min: %s", fmtMin);
            } else if (s_activeRangeThumb == 2) {
                *v_max = std::clamp(newVal, *v_min, max_limit);
                changed = true;
                ImGui::SetTooltip("Max: %s", fmtMax);
            }
        } else if (!held && s_activeRangeId == id) {
            s_activeRangeId = 0;
            s_activeRangeThumb = 0;
        }
    }

    // Spring smooth thumb positions
    t1 = ctx.Anim().SpringFloat(id + 1, (totalRange > 0.001f) ? std::clamp((*v_min - min_limit) / totalRange, 0.0f, 1.0f) : 0.0f, {240.0f, 24.0f, 1.0f});
    t2 = ctx.Anim().SpringFloat(id + 2, (totalRange > 0.001f) ? std::clamp((*v_max - min_limit) / totalRange, 0.0f, 1.0f) : 1.0f, {240.0f, 24.0f, 1.0f});
    thumb1X = trackBb.Min.x + trackBb.GetWidth() * t1;
    thumb2X = trackBb.Min.x + trackBb.GetWidth() * t2;

    // Track background
    drawList->AddRectFilled(trackBb.Min, trackBb.Max, ImColor(colors.CardHover), 3.0f * dpi);
    drawList->AddRect(trackBb.Min, trackBb.Max, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);

    // Active highlighted span between thumb 1 and thumb 2
    if (thumb2X > thumb1X) {
        drawList->AddRectFilled(
            ImVec2(thumb1X, trackBb.Min.y),
            ImVec2(thumb2X, trackBb.Max.y),
            ImColor(disabled ? colors.TextMuted : colors.Accent),
            3.0f * dpi
        );
    }

    // Dynamic radii for dual thumbs
    const float r1 = (s_activeRangeThumb == 1 && held) ? (8.5f * dpi) : (7.0f * dpi);
    const float r2 = (s_activeRangeThumb == 2 && held) ? (8.5f * dpi) : (7.0f * dpi);
    const float midY = trackBb.Min.y + trackBb.GetHeight() * 0.5f;

    // Glow halos when active
    if (s_activeRangeThumb == 1 && held) {
        drawList->AddCircleFilled(ImVec2(thumb1X, midY), r1 + 5.0f * dpi, ImColor(colors.Accent.x, colors.Accent.y, colors.Accent.z, 0.35f));
    }
    if (s_activeRangeThumb == 2 && held) {
        drawList->AddCircleFilled(ImVec2(thumb2X, midY), r2 + 5.0f * dpi, ImColor(colors.Accent.x, colors.Accent.y, colors.Accent.z, 0.35f));
    }

    // Thumb 1 (Min)
    drawList->AddCircleFilled(ImVec2(thumb1X, midY + 1.5f * dpi), r1, IM_COL32(0, 0, 0, 90));
    drawList->AddCircleFilled(ImVec2(thumb1X, midY), r1, IM_COL32(255, 255, 255, 255));
    drawList->AddCircle(ImVec2(thumb1X, midY), r1, ImColor(colors.Accent), 20, 1.5f);

    // Thumb 2 (Max)
    drawList->AddCircleFilled(ImVec2(thumb2X, midY + 1.5f * dpi), r2, IM_COL32(0, 0, 0, 90));
    drawList->AddCircleFilled(ImVec2(thumb2X, midY), r2, IM_COL32(255, 255, 255, 255));
    drawList->AddCircle(ImVec2(thumb2X, midY), r2, ImColor(colors.Accent), 20, 1.5f);

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
    if (RangeSliderFloat(label, &fmin, &fmax, static_cast<float>(min_limit), static_cast<float>(max_limit), format, width, disabled)) {
        *v_min = static_cast<int>(std::round(fmin));
        *v_max = static_cast<int>(std::round(fmax));
        return true;
    }
    return false;
}

} // namespace parallax
