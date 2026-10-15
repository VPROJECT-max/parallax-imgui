#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <imgui.h>

namespace parallax {

enum class NotificationLevel {
    Info,
    Success,
    Warning,
    Danger
};

struct Notification {
    uint32_t id{0};
    NotificationLevel level{NotificationLevel::Info};
    std::string title;
    std::string message;
    float duration{4.0f};
    float elapsed{0.0f};
    float animX{0.0f};     // 0 = hidden offscreen right, 1 = fully visible
    bool dismissed{false};
};

class NotificationManager {
public:
    static NotificationManager& Get() noexcept;

    void Notify(NotificationLevel level, const std::string& title, const std::string& message, float duration = 4.0f);
    void Render();

private:
    NotificationManager() = default;
    std::vector<Notification> m_notifications;
    uint32_t m_nextId{1};
};

// Global convenience wrappers
inline void Notify(NotificationLevel level, const std::string& title, const std::string& message, float duration = 4.0f) {
    NotificationManager::Get().Notify(level, title, message, duration);
}

inline void RenderNotifications() {
    NotificationManager::Get().Render();
}

} // namespace parallax
