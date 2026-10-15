#pragma once

#include <imgui.h>

namespace parallax {

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
