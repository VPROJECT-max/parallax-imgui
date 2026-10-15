#pragma once

#include "parallax/core/types.hpp"
#include "parallax/anim/easing.hpp"
#include "parallax/anim/spring.hpp"
#include <unordered_map>
#include <imgui.h>

namespace parallax::anim {

struct BezierAnimState {
    float startValue{0.0f};
    float targetValue{0.0f};
    float currentValue{0.0f};
    float elapsed{0.0f};
    float duration{0.25f};
    EasingPreset preset{EasingPreset::EaseOutQuint};
    bool active{false};
};

struct SpringAnimState {
    float currentValue{0.0f};
    float velocity{0.0f};
    float targetValue{0.0f};
    SpringConfig config{};
    bool initialized{false};
};

struct Spring2DAnimState {
    ImVec2 currentValue{0.0f, 0.0f};
    ImVec2 velocity{0.0f, 0.0f};
    ImVec2 targetValue{0.0f, 0.0f};
    SpringConfig config{};
    bool initialized{false};
};

struct WindowTransitionState {
    float alpha{0.0f};
    float scale{0.92f};
    float targetState{0.0f}; // 0.0 = closed, 1.0 = open
    bool isVisible{false};
};

class AnimationManager {
public:
    AnimationManager() = default;

    void NewFrame();

    void SetSpeedMultiplier(SpeedMultiplier speed) noexcept { m_speed = speed; }
    SpeedMultiplier GetSpeedMultiplier() const noexcept { return m_speed; }

    float GetEffectiveDeltaTime() const noexcept;

    // Easing-based animation
    float AnimFloat(ImGuiID id, float target, float duration = 0.25f, EasingPreset preset = EasingPreset::EaseOutQuint);
    ImVec4 AnimColor(ImGuiID id, const ImVec4& target, float duration = 0.20f, EasingPreset preset = EasingPreset::EaseOutQuint);

    // Spring physics-based animation
    float SpringFloat(ImGuiID id, float target, const SpringConfig& config = SpringConfig{});
    ImVec2 SpringVec2(ImGuiID id, const ImVec2& target, const SpringConfig& config = SpringConfig{});

    // Window open/close transition
    void UpdateOpenCloseTransition(bool requestOpen, float& outAlpha, float& outScale, bool& outShouldRender);

private:
    SpeedMultiplier m_speed{SpeedMultiplier::Normal};
    float m_dt{0.016f};

    std::unordered_map<ImGuiID, BezierAnimState> m_bezierStates;
    std::unordered_map<ImGuiID, SpringAnimState> m_springStates;
    std::unordered_map<ImGuiID, Spring2DAnimState> m_spring2DStates;

    WindowTransitionState m_windowTransition;
};

} // namespace parallax::anim
