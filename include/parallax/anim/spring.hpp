#pragma once

#include <imgui.h>

namespace parallax::anim {

struct SpringConfig {
    float stiffness{100.0f};  // Tension / spring constant k
    float damping{20.0f};     // Friction / damping coefficient c
    float mass{1.0f};         // Mass m
    float precision{0.0005f}; // Settling threshold
};

class SpringSolver {
public:
    static void Step(float& current, float& velocity, float target, float dt, const SpringConfig& config) noexcept;
    static void Step2D(ImVec2& current, ImVec2& velocity, const ImVec2& target, float dt, const SpringConfig& config) noexcept;
};

} // namespace parallax::anim
