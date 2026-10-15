#pragma once

#include <cstdint>

namespace parallax::anim {

enum class EasingPreset : uint8_t {
    Linear = 0,
    EaseOutQuart,
    EaseOutQuint,
    EaseOutExpo,
    EaseInOutStandard,
    SheetStandard
};

class CubicBezier {
public:
    constexpr CubicBezier(float x1, float y1, float x2, float y2) noexcept
        : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}

    float Solve(float x) const noexcept;

    static const CubicBezier& GetPreset(EasingPreset preset) noexcept;

private:
    float SampleCurveX(float t) const noexcept;
    float SampleCurveY(float t) const noexcept;
    float SampleCurveDerivativeX(float t) const noexcept;
    float SolveCurveX(float x) const noexcept;

    float m_x1{0.25f};
    float m_y1{1.00f};
    float m_x2{0.50f};
    float m_y2{1.00f};
};

} // namespace parallax::anim
