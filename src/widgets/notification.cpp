#include "parallax/widgets/notification.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/anim/spring.hpp"
#include <algorithm>
#include <cstdio>

namespace parallax {

NotificationManager& NotificationManager::Get() noexcept {
    static NotificationManager instance;
    return instance;
}

void NotificationManager::Notify(NotificationLevel level, const std::string& title, const std::string& message, float duration) {
    Notification notif;
    notif.id = m_nextId++;
    notif.level = level;
    notif.title = title;
    notif.message = message;
    notif.duration = duration > 0.5f ? duration : 4.0f;
    notif.elapsed = 0.0f;
    notif.animX = 0.0f;
    notif.dismissed = false;

    m_notifications.push_back(std::move(notif));
}

void NotificationManager::Render() {
    if (m_notifications.empty()) return;

    ImGuiIO& io = ImGui::GetIO();
    const float dt = io.DeltaTime;
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();

    const float cardWidth = 320.0f * dpi;
    const float cardHeight = 68.0f * dpi;
    const float marginX = 24.0f * dpi;
    const float marginY = 24.0f * dpi;
    const float gap = 10.0f * dpi;

    float currentY = io.DisplaySize.y - marginY;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        Notification& notif = *it;
        notif.elapsed += dt;

        if (notif.elapsed >= notif.duration) {
            notif.dismissed = true;
        }

        char notifIdStr[64];
        snprintf(notifIdStr, sizeof(notifIdStr), "##NotifSpring_%u", notif.id);
        const ImGuiID springId = ImGui::GetID(notifIdStr);

        const float targetX = notif.dismissed ? 0.0f : 1.0f;
        notif.animX = ctx.Anim().SpringFloat(springId, targetX, {140.0f, 20.0f, 1.0f, 0.001f});

        if (notif.dismissed && notif.animX < 0.02f) {
            it = m_notifications.erase(it);
            continue;
        }

        currentY -= cardHeight;

        // Animate from off-screen right
        const float targetLeft = io.DisplaySize.x - marginX - cardWidth;
        const float offscreenLeft = io.DisplaySize.x + 20.0f;
        const float posX = offscreenLeft + (targetLeft - offscreenLeft) * notif.animX;
        const float posY = currentY;

        const ImVec2 minPos(posX, posY);
        const ImVec2 maxPos(posX + cardWidth, posY + cardHeight);
        const float rounding = metrics.FrameRounding * dpi;

        // Determine level color and icon name
        ImVec4 levelCol = colors.Accent;
        const char* iconName = "bell";

        switch (notif.level) {
            case NotificationLevel::Success:
                levelCol = colors.Success;
                iconName = "check";
                break;
            case NotificationLevel::Warning:
                levelCol = colors.Warning;
                iconName = "alert-triangle";
                break;
            case NotificationLevel::Danger:
                levelCol = colors.Danger;
                iconName = "alert-circle";
                break;
            case NotificationLevel::Info:
            default:
                levelCol = colors.Accent;
                iconName = "bell";
                break;
        }

        // Drop shadow
        drawList->AddRectFilled(
            ImVec2(minPos.x + 2.0f, minPos.y + 4.0f),
            ImVec2(maxPos.x + 2.0f, maxPos.y + 4.0f),
            IM_COL32(0, 0, 0, 70),
            rounding
        );

        // Card background
        drawList->AddRectFilled(minPos, maxPos, IM_COL32(18, 20, 31, 248), rounding);
        drawList->AddRect(minPos, maxPos, ImColor(colors.Border), rounding, 0, 1.0f);

        // Left accent strip
        drawList->AddRectFilled(
            minPos,
            ImVec2(minPos.x + 4.0f * dpi, maxPos.y),
            ImColor(levelCol),
            rounding,
            ImDrawFlags_RoundCornersLeft
        );

        // Icon
        const float iconSize = 20.0f * dpi;
        const ImVec2 iconPos(minPos.x + 14.0f * dpi, minPos.y + (cardHeight - iconSize) * 0.5f);
        ctx.Icons().DrawIcon(iconName, ImVec2(iconSize, iconSize), levelCol);

        // Title
        PushFont(FontWeight::SemiBold);
        drawList->AddText(
            ImVec2(minPos.x + 44.0f * dpi, minPos.y + 12.0f * dpi),
            ImColor(colors.TextPrimary),
            notif.title.c_str()
        );
        PopFont();

        // Message
        PushFont(FontWeight::Caption);
        drawList->AddText(
            ImVec2(minPos.x + 44.0f * dpi, minPos.y + 34.0f * dpi),
            ImColor(colors.TextSecondary),
            notif.message.c_str()
        );
        PopFont();

        // Remaining time progress bar along bottom
        const float progress = std::clamp(1.0f - (notif.elapsed / notif.duration), 0.0f, 1.0f);
        if (progress > 0.0f && !notif.dismissed) {
            drawList->AddRectFilled(
                ImVec2(minPos.x + 4.0f * dpi, maxPos.y - 2.0f * dpi),
                ImVec2(minPos.x + 4.0f * dpi + (cardWidth - 4.0f * dpi) * progress, maxPos.y),
                ImColor(levelCol.x, levelCol.y, levelCol.z, 0.6f),
                rounding,
                ImDrawFlags_RoundCornersBottom
            );
        }

        currentY -= gap;
        ++it;
    }
}

} // namespace parallax
