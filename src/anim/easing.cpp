#include "parallax/anim/easing.hpp"
#include <cmath>
#include <algorithm>

namespace parallax::anim {

float CubicBezier::SampleCurveX(float t) const noexcept {
    // ((1-3*x2+3*x1)*t + (3*x2-6*x1))*t + 3*x1)*t
    return ((1.0f - 3.0f * m_x2 + 3.0f * m_x1) * t + (3.0f * m_x2 - 6.0f * m_x1)) * t * t + 3.0f * m_x1 * t;
}

float CubicBezier::SampleCurveY(float t) const noexcept {
    return ((1.0f - 3.0f * m_y2 + 3.0f * m_y1) * t + (3.0f * m_y2 - 6.0f * m_y1)) * t * t + 3.0f * m_y1 * t;
}

float CubicBezier::SampleCurveDerivativeX(float t) const noexcept {
    return (3.0f * (1.0f - 3.0f * m_x2 + 3.0f * m_x1) * t + 2.0f * (3.0f * m_x2 - 6.0f * m_x1)) * t + 3.0f * m_x1;
}

float CubicBezier::SolveCurveX(float x) const noexcept {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;

    // First try Newton-Raphson iteration
    float t2 = x;
    for (int i = 0; i < 8; ++i) {
        float x2 = SampleCurveX(t2) - x;
        if (std::abs(x2) < 1e-5f)
            return t2;
        float d2 = SampleCurveDerivativeX(t2);
        if (std::abs(d2) < 1e-5f)
            break;
        t2 -= x2 / d2;
    }

    // Fall back to bisection if Newton's method fails
    float t0 = 0.0f;
    float t1 = 1.0f;
    t2 = x;

    while (t0 < t1) {
        float x2 = SampleCurveX(t2);
        if (std::abs(x2 - x) < 1e-5f)
            return t2;
        if (x > x2)
            t0 = t2;
        else
            t1 = t2;
        t2 = (t1 - t0) * 0.5f + t0;
    }

    return t2;
}

float CubicBezier::Solve(float x) const noexcept {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    return SampleCurveY(SolveCurveX(x));
}

const CubicBezier& CubicBezier::GetPreset(EasingPreset preset) noexcept {
    static const CubicBezier s_linear(0.0f, 0.0f, 1.0f, 1.0f);
    static const CubicBezier s_easeOutQuart(0.25f, 1.0f, 0.50f, 1.0f);
    static const CubicBezier s_easeOutQuint(0.22f, 1.0f, 0.36f, 1.0f);
    static const CubicBezier s_easeOutExpo(0.16f, 1.0f, 0.30f, 1.0f);
    static const CubicBezier s_easeInOut(0.65f, 0.0f, 0.35f, 1.0f);
    static const CubicBezier s_sheet(0.32f, 0.72f, 0.0f, 1.0f);

    switch (preset) {
        case EasingPreset::Linear:            return s_linear;
        case EasingPreset::EaseOutQuart:      return s_easeOutQuart;
        case EasingPreset::EaseOutQuint:      return s_easeOutQuint;
        case EasingPreset::EaseOutExpo:       return s_easeOutExpo;
        case EasingPreset::EaseInOutStandard: return s_easeInOut;
        case EasingPreset::SheetStandard:     return s_sheet;
    }
    return s_easeOutQuint;
}

} // namespace parallax::anim
