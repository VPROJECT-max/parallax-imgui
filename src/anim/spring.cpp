#include "parallax/anim/spring.hpp"
#include <cmath>
#include <algorithm>

namespace parallax::anim {

void SpringSolver::Step(float& current, float& velocity, float target, float dt, const SpringConfig& config) noexcept {
    if (dt <= 0.0f) return;

    // Sub-step for numerical stability (maximum dt per step: 1/120 sec)
    constexpr float maxStepDt = 1.0f / 120.0f;
    float remaining = std::min(dt, 0.1f); // Cap to 100ms to avoid physics explosions on lag spikes

    while (remaining > 0.0f) {
        const float step = std::min(remaining, maxStepDt);
        
        // Semi-implicit Euler integration:
        // F = -k * (x - target) - c * v
        const float force = -config.stiffness * (current - target) - config.damping * velocity;
        const float accel = force / (config.mass > 0.001f ? config.mass : 1.0f);
        
        velocity += accel * step;
        current += velocity * step;
        
        remaining -= step;
    }

    // Snap to target if settled within precision threshold
    if (std::abs(current - target) < config.precision && std::abs(velocity) < (config.precision * 10.0f)) {
        current = target;
        velocity = 0.0f;
    }
}

void SpringSolver::Step2D(ImVec2& current, ImVec2& velocity, const ImVec2& target, float dt, const SpringConfig& config) noexcept {
    Step(current.x, velocity.x, target.x, dt, config);
    Step(current.y, velocity.y, target.y, dt, config);
}

} // namespace parallax::anim
