#pragma once

#include "parallax/widgets/checkbox.hpp"
#include "parallax/widgets/keybind.hpp"
#include "parallax/widgets/color_picker.hpp"
#include <functional>
#include <string>
#include <imgui.h>

namespace parallax {

class ControlRow {
public:
    ControlRow(const char* label, bool* p_value);
    ~ControlRow();

    // Move semantics for safe chaining
    ControlRow(ControlRow&& other) noexcept;
    ControlRow& operator=(ControlRow&& other) noexcept;

    ControlRow(const ControlRow&) = delete;
    ControlRow& operator=(const ControlRow&) = delete;

    // Fluent Chaining Methods
    ControlRow& Icon(const char* iconName) noexcept;
    ControlRow& Description(const char* desc) noexcept;
    ControlRow& Tooltip(const char* tip) noexcept;
    ControlRow& Disabled(bool disabled = true) noexcept;
    ControlRow& Style(ToggleStyle style) noexcept;
    ControlRow& Keybind(parallax::Keybind* keybind) noexcept;
    ControlRow& Color(ImVec4* color, bool hasAlpha = true) noexcept;
    ControlRow& Color(float* color3or4, bool hasAlpha = true) noexcept;
    ControlRow& OnChange(std::function<void(bool)> callback) noexcept;

    // Explicit render if caller wants immediate return value
    bool Render();

private:
    const char* m_label{nullptr};
    bool* m_value{nullptr};
    const char* m_icon{nullptr};
    const char* m_description{nullptr};
    const char* m_tooltip{nullptr};
    bool m_disabled{false};
    ToggleStyle m_style{ToggleStyle::PillSwitch};

    parallax::Keybind* m_keybind{nullptr};
    ImVec4* m_colorVec4{nullptr};
    float* m_colorFloat{nullptr};
    bool m_hasAlpha{true};

    std::function<void(bool)> m_callback{nullptr};
    bool m_rendered{false};
};

} // namespace parallax
