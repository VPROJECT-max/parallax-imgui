#include "parallax/input/hotkeys.hpp"
#include <imgui.h>

namespace parallax::input {

InputHandler::InputHandler() {
    ResetToDefaults();
}

void InputHandler::ResetToDefaults() {
    m_toggleKeys.clear();
    // Default 1: Right Shift (typing safety enabled)
    m_toggleKeys.push_back({VK_RSHIFT, true});
    // Default 2: Insert (typing safety disabled, always toggles)
    m_toggleKeys.push_back({VK_INSERT, false});
}

void InputHandler::AddToggleKey(int vkey, bool ignoreWhenTyping) {
    RemoveToggleKey(vkey);
    m_toggleKeys.push_back({vkey, ignoreWhenTyping});
}

void InputHandler::RemoveToggleKey(int vkey) {
    std::erase_if(m_toggleKeys, [vkey](const ToggleKey& tk) {
        return tk.vkey == vkey;
    });
}

void InputHandler::ClearToggleKeys() {
    m_toggleKeys.clear();
}

void InputHandler::Update() {
    const bool isTyping = ImGui::GetIO().WantTextInput;

    for (const auto& tk : m_toggleKeys) {
        const SHORT keyState = GetAsyncKeyState(tk.vkey);
        const bool isDown = (keyState & 0x8000) != 0;
        const bool wasDown = m_wasKeyDown[tk.vkey];
        m_wasKeyDown[tk.vkey] = isDown;

        // Trigger on transition edge (key down)
        if (isDown && !wasDown) {
            // Typing safety check: ignore if user is currently typing in an ImGui text field
            if (tk.ignoreWhenTyping && isTyping) {
                continue;
            }

            ToggleMenu();
            if (m_onToggle) {
                m_onToggle(m_menuOpen);
            }
            break; // Handled toggle this frame
        }
    }
}

} // namespace parallax::input
