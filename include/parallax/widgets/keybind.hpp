#pragma once

#include <string>
#include <cstdint>
#include <imgui.h>

namespace parallax {

enum class KeybindMode : uint8_t {
    Hold = 0,
    Toggle,
    AlwaysOn
};

struct Keybind {
    int vkey{0};                          // Virtual-key code (0 = NONE)
    KeybindMode mode{KeybindMode::Toggle};
    bool active{false};                   // Live computed state
};

// Returns a human-friendly string for a virtual-key code (e.g. "F", "M4", "SPACE", "NONE")
std::string GetKeyName(int vkey);

// Evaluates whether the keybind is currently triggered / active
bool UpdateKeybindState(Keybind& bind);

// Testing and headless input simulation helpers
void SimulateKeyState(int vkey, bool isDown);
void ClearSimulatedKeys();

// Inline keybind button widget with anchored configuration popup
bool KeybindWidget(const char* str_id, Keybind* bind, const ImVec2& size = ImVec2(0.0f, 0.0f));

// First-class row containing label, description, and right-aligned keybind button
bool KeybindRow(const char* label, const char* description, Keybind* bind);

} // namespace parallax

