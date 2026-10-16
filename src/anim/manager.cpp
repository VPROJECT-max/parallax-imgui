#include "parallax/anim/manager.hpp"
#include <algorithm>
#include <cmath>

namespace parallax::anim {

void AnimationManager::NewFrame() {
    float ioDt = ImGui::GetIO().DeltaTime;
    if (ioDt <= 0.0f || ioDt > 0.2f) {
        ioDt = 1.0f / 60.0f;
    }
    m_dt = ioDt;
}

float AnimationManager::GetEffectiveDeltaTime() const noexcept {
    switch (m_speed) {
        case SpeedMultiplier::Fast:    return m_dt * 1.5f;
        case SpeedMultiplier::Normal:  return m_dt * 1.0f;
        case SpeedMultiplier::Relaxed: return m_dt * 0.7f;
        case SpeedMultiplier::Off:     return 0.0f;
    }
    return m_dt;
}

float AnimationManager::AnimFloat(ImGuiID id, float target, float duration, EasingPreset preset) {
    if (m_speed == SpeedMultiplier::Off || duration <= 0.001f) {
        return target;
    }

    auto& state = m_bezierStates[id];
    if (!state.initialized) {
        state.currentValue = target;
        state.startValue = target;
        state.targetValue = target;
        state.initialized = true;
        return target;
    }

    if (std::abs(state.targetValue - target) > 0.0001f) {
        state.startValue = state.currentValue;
        state.targetValue = target;
        state.elapsed = 0.0f;
        state.duration = std::max(duration, 0.01f);
        state.preset = preset;
        state.active = true;
    }

    if (state.active) {
        float effDt = GetEffectiveDeltaTime();
        state.elapsed += effDt;
        float progress = std::clamp(state.elapsed / state.duration, 0.0f, 1.0f);
        float t = CubicBezier::GetPreset(state.preset).Solve(progress);
        state.currentValue = state.startValue + (state.targetValue - state.startValue) * t;

        if (progress >= 1.0f) {
            state.currentValue = state.targetValue;
            state.active = false;
        }
    }

    return state.currentValue;
}

ImVec4 AnimationManager::AnimColor(ImGuiID id, const ImVec4& target, float duration, EasingPreset preset) {
    float r = AnimFloat(id + 1, target.x, duration, preset);
    float g = AnimFloat(id + 2, target.y, duration, preset);
    float b = AnimFloat(id + 3, target.z, duration, preset);
    float a = AnimFloat(id + 4, target.w, duration, preset);
    return ImVec4(r, g, b, a);
}

float AnimationManager::SpringFloat(ImGuiID id, float target, const SpringConfig& config) {
    if (m_speed == SpeedMultiplier::Off) {
        return target;
    }

    auto& state = m_springStates[id];
    if (!state.initialized) {
        state.currentValue = target;
        state.velocity = 0.0f;
        state.targetValue = target;
        state.config = config;
        state.initialized = true;
        return target;
    }

    state.targetValue = target;
    state.config = config;

    float effDt = GetEffectiveDeltaTime();
    SpringSolver::Step(state.currentValue, state.velocity, state.targetValue, effDt, state.config);
    return state.currentValue;
}

ImVec2 AnimationManager::SpringVec2(ImGuiID id, const ImVec2& target, const SpringConfig& config) {
    if (m_speed == SpeedMultiplier::Off) {
        return target;
    }

    auto& state = m_spring2DStates[id];
    if (!state.initialized) {
        state.currentValue = target;
        state.velocity = ImVec2(0.0f, 0.0f);
        state.targetValue = target;
        state.config = config;
        state.initialized = true;
        return target;
    }

    state.targetValue = target;
    state.config = config;

    float effDt = GetEffectiveDeltaTime();
    SpringSolver::Step2D(state.currentValue, state.velocity, state.targetValue, effDt, state.config);
    return state.currentValue;
}

void AnimationManager::UpdateOpenCloseTransition(bool requestOpen, float& outAlpha, float& outScale, bool& outShouldRender) {
    if (m_speed == SpeedMultiplier::Off) {
        outAlpha = requestOpen ? 1.0f : 0.0f;
        outScale = 1.0f;
        outShouldRender = requestOpen;
        m_windowTransition.alpha = outAlpha;
        m_windowTransition.scale = outScale;
        m_windowTransition.isVisible = requestOpen;
        return;
    }

    float targetAlpha = requestOpen ? 1.0f : 0.0f;
    float targetScale = requestOpen ? 1.0f : 0.92f;

    // Use a snappy damped spring for window open/close
    SpringConfig springCfg{120.0f, 20.0f, 1.0f, 0.001f};
    static float velocityAlpha = 0.0f;
    static float velocityScale = 0.0f;

    float effDt = GetEffectiveDeltaTime();
    SpringSolver::Step(m_windowTransition.alpha, velocityAlpha, targetAlpha, effDt, springCfg);
    SpringSolver::Step(m_windowTransition.scale, velocityScale, targetScale, effDt, springCfg);

    // Keep clamped and stable
    m_windowTransition.alpha = std::clamp(m_windowTransition.alpha, 0.0f, 1.0f);
    m_windowTransition.scale = std::clamp(m_windowTransition.scale, 0.90f, 1.02f);

    outAlpha = m_windowTransition.alpha;
    outScale = m_windowTransition.scale;

    // Render if alpha > 0.001f or if requestOpen is true
    outShouldRender = requestOpen || (outAlpha > 0.002f);
    m_windowTransition.isVisible = outShouldRender;
}

} // namespace parallax::anim
