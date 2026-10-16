#pragma once

#include <imgui.h>

namespace parallax {

struct SliderOptions {
    const char* format{"%.2f"};
    float width{0.0f};
    bool twoLine{false};
    bool compact{false};
    bool disabled{false};
    int tickCount{5};           // Number of subtle reference ticks along the track (0 = none)
    bool floatingValue{true};   // Show floating value callout above thumb while dragging
};

bool SliderFloat(
    const char* label,
    float* v,
    float v_min,
    float v_max,
    const char* format = "%.2f",
    float width = 0.0f,
    bool twoLine = false,
    bool disabled = false
);

bool SliderFloatEx(
    const char* label,
    float* v,
    float v_min,
    float v_max,
    const SliderOptions& options
);

bool SliderInt(
    const char* label,
    int* v,
    int v_min,
    int v_max,
    const char* format = "%d",
    float width = 0.0f,
    bool twoLine = false,
    bool disabled = false
);

bool RangeSliderFloat(
    const char* label,
    float* v_min,
    float* v_max,
    float min_limit,
    float max_limit,
    const char* format = "%.2f",
    float width = 0.0f,
    bool disabled = false
);

bool RangeSliderInt(
    const char* label,
    int* v_min,
    int* v_max,
    int min_limit,
    int max_limit,
    const char* format = "%d",
    float width = 0.0f,
    bool disabled = false
);

} // namespace parallax
