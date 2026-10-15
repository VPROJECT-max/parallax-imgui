#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <windows.h>

namespace parallax::input {

struct ToggleKey {
    int vkey{VK_RSHIFT};
    bool ignoreWhenTyping{true};
};

class InputHandler {
public:
    InputHandler();

    void Update();

    bool IsMenuOpen() const noexcept { return m_menuOpen; }
    void SetMenuOpen(bool open) noexcept { m_menuOpen = open; }
    void ToggleMenu() noexcept { m_menuOpen = !m_menuOpen; }

    void AddToggleKey(int vkey, bool ignoreWhenTyping = true);
    void RemoveToggleKey(int vkey);
    void ClearToggleKeys();
    void ResetToDefaults();

    const std::vector<ToggleKey>& GetToggleKeys() const noexcept { return m_toggleKeys; }

    using OnToggleCallback = std::function<void(bool open)>;
    void SetOnToggleCallback(OnToggleCallback callback) { m_onToggle = std::move(callback); }

    void SetKeyCaptureActive(bool active) noexcept { m_keyCaptureActive = active; }
    bool IsKeyCaptureActive() const noexcept { return m_keyCaptureActive; }

private:
    bool m_menuOpen{true};
    bool m_keyCaptureActive{false};
    std::vector<ToggleKey> m_toggleKeys;
    std::unordered_map<int, bool> m_wasKeyDown;
    OnToggleCallback m_onToggle;
};

} // namespace parallax::input
