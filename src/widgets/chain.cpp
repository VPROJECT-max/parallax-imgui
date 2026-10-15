#include "parallax/widgets/chain.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <utility>

namespace parallax {

ControlRow::ControlRow(const char* label, bool* p_value)
    : m_label(label), m_value(p_value) {}

ControlRow::~ControlRow() {
    if (!m_rendered && m_value) {
        Render();
    }
}

ControlRow::ControlRow(ControlRow&& other) noexcept {
    m_label = other.m_label;
    m_value = other.m_value;
    m_icon = other.m_icon;
    m_description = other.m_description;
    m_tooltip = other.m_tooltip;
    m_disabled = other.m_disabled;
    m_style = other.m_style;
    m_keybind = other.m_keybind;
    m_colorVec4 = other.m_colorVec4;
    m_colorFloat = other.m_colorFloat;
    m_hasAlpha = other.m_hasAlpha;
    m_callback = std::move(other.m_callback);
    m_rendered = other.m_rendered;

    other.m_rendered = true; // Prevent moved-from object from rendering
}

ControlRow& ControlRow::operator=(ControlRow&& other) noexcept {
    if (this != &other) {
        if (!m_rendered && m_value) {
            Render();
        }

        m_label = other.m_label;
        m_value = other.m_value;
        m_icon = other.m_icon;
        m_description = other.m_description;
        m_tooltip = other.m_tooltip;
        m_disabled = other.m_disabled;
        m_style = other.m_style;
        m_keybind = other.m_keybind;
        m_colorVec4 = other.m_colorVec4;
        m_colorFloat = other.m_colorFloat;
        m_hasAlpha = other.m_hasAlpha;
        m_callback = std::move(other.m_callback);
        m_rendered = other.m_rendered;

        other.m_rendered = true;
    }
    return *this;
}

ControlRow& ControlRow::Icon(const char* iconName) noexcept {
    m_icon = iconName;
    return *this;
}

ControlRow& ControlRow::Description(const char* desc) noexcept {
    m_description = desc;
    return *this;
}

ControlRow& ControlRow::Tooltip(const char* tip) noexcept {
    m_tooltip = tip;
    return *this;
}

ControlRow& ControlRow::Disabled(bool disabled) noexcept {
    m_disabled = disabled;
    return *this;
}

ControlRow& ControlRow::Style(ToggleStyle style) noexcept {
    m_style = style;
    return *this;
}

ControlRow& ControlRow::Keybind(parallax::Keybind* keybind) noexcept {
    m_keybind = keybind;
    return *this;
}

ControlRow& ControlRow::Color(ImVec4* color, bool hasAlpha) noexcept {
    m_colorVec4 = color;
    m_hasAlpha = hasAlpha;
    return *this;
}

ControlRow& ControlRow::Color(float* color3or4, bool hasAlpha) noexcept {
    m_colorFloat = color3or4;
    m_hasAlpha = hasAlpha;
    return *this;
}

ControlRow& ControlRow::OnChange(std::function<void(bool)> callback) noexcept {
    m_callback = std::move(callback);
    return *this;
}

bool ControlRow::Render() {
    if (m_rendered || !m_value) return false;
    m_rendered = true;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(m_label ? m_label : "##ControlRow");
    const float availW = ImGui::GetContentRegionAvail().x;
    const bool hasDesc = (m_description && m_description[0] != '\0');
    const float rowHeight = hasDesc ? (38.0f * dpi) : (28.0f * dpi);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowHeight));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    // Right-aligned widgets width calculation
    const float switchW = (m_style == ToggleStyle::PillSwitch) ? (34.0f * dpi) : (18.0f * dpi);
    float rightOffset = 4.0f * dpi;

    // Rightmost slot: Primary toggle
    const float switchX = bb.Max.x - switchW - rightOffset;
    rightOffset += switchW + 8.0f * dpi;

    // Color swatch slot
    float colorX = 0.0f;
    const float colorDim = 22.0f * dpi;
    if (m_colorVec4 || m_colorFloat) {
        colorX = bb.Max.x - colorDim - rightOffset;
        rightOffset += colorDim + 8.0f * dpi;
    }

    // Keybind slot
    float kbX = 0.0f;
    const float kbW = 48.0f * dpi;
    const float kbH = 22.0f * dpi;
    if (m_keybind) {
        kbX = bb.Max.x - kbW - rightOffset;
        rightOffset += kbW + 8.0f * dpi;
    }

    // Calculate bounds of secondary controls so clicking the toggle doesn't interfere
    float minSecondaryX = switchX;
    if (m_colorVec4 || m_colorFloat) minSecondaryX = std::min(minSecondaryX, colorX);
    if (m_keybind) minSecondaryX = std::min(minSecondaryX, kbX);

    // Two clickable regions for the toggle: the label area and the switch itself
    const ImRect labelClickBb(bb.Min, ImVec2(minSecondaryX - 4.0f * dpi, bb.Max.y));
    const ImRect switchClickBb(ImVec2(switchX - 4.0f * dpi, bb.Min.y), ImVec2(bb.Max.x, bb.Max.y));

    bool hovered = false;
    bool held = false;
    bool pressed = false;

    if (!m_disabled) {
        ImVec2 mouse = ImGui::GetIO().MousePos;
        bool inLabel = labelClickBb.Contains(mouse);
        bool inSwitch = switchClickBb.Contains(mouse);
        if (inLabel || inSwitch || ImGui::GetActiveID() == id) {
            ImRect activeHitBb = inSwitch ? switchClickBb : labelClickBb;
            pressed = ImGui::ButtonBehavior(activeHitBb, id, &hovered, &held);
        }
    }

    if (pressed) {
        *m_value = !(*m_value);
        if (m_callback) {
            m_callback(*m_value);
        }
    }

    // Animated toggle progress
    const float targetCheck = *m_value ? 1.0f : 0.0f;
    const float checkProgress = ctx.Anim().SpringFloat(id, targetCheck, {200.0f, 22.0f, 1.0f});

    // Row hover background highlight
    if (hovered && !m_disabled) {
        drawList->AddRectFilled(
            ImVec2(bb.Min.x - 4.0f * dpi, bb.Min.y - 2.0f * dpi),
            ImVec2(bb.Max.x + 4.0f * dpi, bb.Max.y + 2.0f * dpi),
            ImColor(colors.CardHover),
            metrics.FrameRounding
        );
    }

    // Left side: Icon + Label + Description
    float textX = bb.Min.x + 4.0f * dpi;
    if (m_icon && m_icon[0] != '\0') {
        const float iconDim = 16.0f * dpi;
        const ImVec2 iconPos = ImVec2(textX, bb.Min.y + (hasDesc ? 6.0f * dpi : (rowHeight - iconDim) * 0.5f));
        const ImVec4 iconCol = m_disabled ? colors.TextMuted : (hovered ? colors.Accent : colors.TextSecondary);
        ctx.Icons().DrawIcon(drawList, m_icon, iconPos, ImVec2(iconDim, iconDim), iconCol);
        textX += iconDim + 8.0f * dpi;
    }

    ImVec4 labelCol = m_disabled ? colors.TextMuted : colors.TextPrimary;
    const float labelY = hasDesc ? (bb.Min.y + 4.0f * dpi) : (bb.Min.y + (rowHeight - ImGui::GetFontSize()) * 0.5f);
    PushFont(FontWeight::Medium);
    const char* textEnd = ImGui::FindRenderedTextEnd(m_label);
    drawList->AddText(nullptr, 0.0f, ImVec2(textX, labelY), ImColor(labelCol), m_label, textEnd);
    PopFont();

    if (hasDesc) {
        const float descY = labelY + ImGui::GetFontSize() + 2.0f * dpi;
        PushFont(FontWeight::Caption);
        drawList->AddText(ImVec2(textX, descY), ImColor(colors.TextMuted), m_description);
        PopFont();
    }

    // Render Chained Secondary Widgets:
    // 1. Inline Keybind
    if (m_keybind) {
        ImGui::SetCursorScreenPos(ImVec2(kbX, bb.Min.y + (rowHeight - kbH) * 0.5f));
        char kbId[64];
        snprintf(kbId, sizeof(kbId), "##RowKB_%08X", id);
        KeybindWidget(kbId, m_keybind, ImVec2(kbW, kbH));
    }

    // 2. Inline Color Swatch
    if (m_colorVec4 || m_colorFloat) {
        ImGui::SetCursorScreenPos(ImVec2(colorX, bb.Min.y + (rowHeight - colorDim) * 0.5f));
        char colId[64];
        snprintf(colId, sizeof(colId), "##RowCol_%08X", id);
        if (m_colorVec4) {
            ColorPicker(colId, m_colorVec4, m_hasAlpha, ImVec2(colorDim, colorDim));
        } else if (m_colorFloat) {
            ColorPicker(colId, m_colorFloat, m_hasAlpha, ImVec2(colorDim, colorDim));
        }
    }

    // 3. Primary Switch / Checkbox
    if (m_style == ToggleStyle::PillSwitch) {
        const float switchH = 18.0f * dpi;
        const float switchY = bb.Min.y + (rowHeight - switchH) * 0.5f;
        const ImVec2 switchMin = ImVec2(switchX, switchY);
        const ImVec2 switchMax = ImVec2(switchMin.x + switchW, switchMin.y + switchH);

        ImVec4 pillBg = ImVec4(
            colors.CardBg.x + (colors.Accent.x - colors.CardBg.x) * checkProgress,
            colors.CardBg.y + (colors.Accent.y - colors.CardBg.y) * checkProgress,
            colors.CardBg.z + (colors.Accent.z - colors.CardBg.z) * checkProgress,
            m_disabled ? 0.35f : 1.0f
        );

        drawList->AddRectFilled(switchMin, switchMax, ImColor(pillBg), switchH * 0.5f);
        drawList->AddRect(switchMin, switchMax, ImColor(colors.Border), switchH * 0.5f, 0, 1.0f);

        const float thumbRadius = 6.5f * dpi;
        const float startThumbX = switchMin.x + thumbRadius + 2.5f * dpi;
        const float endThumbX = switchMax.x - thumbRadius - 2.5f * dpi;
        const float curThumbX = startThumbX + (endThumbX - startThumbX) * checkProgress;
        const ImVec2 thumbCenter = ImVec2(curThumbX, switchMin.y + switchH * 0.5f);

        drawList->AddCircleFilled(ImVec2(thumbCenter.x, thumbCenter.y + 1.0f * dpi), thumbRadius, IM_COL32(0, 0, 0, 60));
        drawList->AddCircleFilled(thumbCenter, thumbRadius, m_disabled ? ImU32(ImColor(colors.TextMuted)) : IM_COL32(255, 255, 255, 255));
    } else {
        const float boxDim = 18.0f * dpi;
        const float boxY = bb.Min.y + (rowHeight - boxDim) * 0.5f;
        const ImVec2 boxMin = ImVec2(switchX, boxY);
        const ImVec2 boxMax = ImVec2(boxMin.x + boxDim, boxMin.y + boxDim);

        ImVec4 boxBg = ImVec4(
            colors.CardBg.x + (colors.Accent.x - colors.CardBg.x) * checkProgress,
            colors.CardBg.y + (colors.Accent.y - colors.CardBg.y) * checkProgress,
            colors.CardBg.z + (colors.Accent.z - colors.CardBg.z) * checkProgress,
            m_disabled ? 0.35f : 1.0f
        );

        drawList->AddRectFilled(boxMin, boxMax, ImColor(boxBg), 4.0f * dpi);
        drawList->AddRect(boxMin, boxMax, ImColor(hovered ? colors.BorderStrong : colors.Border), 4.0f * dpi, 0, 1.0f);

        if (checkProgress > 0.05f) {
            const ImVec2 p1 = ImVec2(boxMin.x + 4.5f * dpi, boxMin.y + 9.5f * dpi);
            const ImVec2 p2 = ImVec2(boxMin.x + 7.5f * dpi, boxMin.y + 13.0f * dpi);
            const ImVec2 p3 = ImVec2(boxMin.x + 13.5f * dpi, boxMin.y + 5.5f * dpi);

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

    if (m_tooltip && hovered) {
        ImGui::SetTooltip("%s", m_tooltip);
    }

    // Ensure cursor is restored to the end of the entire row so chained sub-widgets do not corrupt layout
    ImGui::SetCursorScreenPos(ImVec2(bb.Min.x, bb.Max.y + ImGui::GetStyle().ItemSpacing.y));

    return pressed;
}

} // namespace parallax
