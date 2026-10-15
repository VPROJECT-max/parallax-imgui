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
    if (m_keyCaptureActive) {
        return;
    }

    const bool isTyping = ImGui::GetIO().WantTextInput;

    for (const auto& tk : m_toggleKeys) {
        bool isDown = false;
        if (tk.vkey == VK_RSHIFT) {
            isDown = ((GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0) ||
                     (((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) && ((GetKeyState(VK_RSHIFT) & 0x8000) != 0));
        } else {
            isDown = (GetAsyncKeyState(tk.vkey) & 0x8000) != 0;
        }

        const bool wasDown = m_wasKeyDown[tk.vkey];
        m_wasKeyDown[tk.vkey] = isDown;

        // Trigger on transition edge (fresh key down)
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
