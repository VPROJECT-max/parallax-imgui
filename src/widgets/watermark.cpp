#include "parallax/widgets/watermark.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include <imgui.h>
#include <cstdio>

namespace parallax {

void Watermark(const char* appName, const char* user, bool showFps, bool showPing) {
    ImGuiIO& io = ImGui::GetIO();
    const float dpi = Context::Get().GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();

    char statsBuf[128];
    const float fps = io.Framerate;
    const float ms = 1000.0f / (fps > 0.0f ? fps : 60.0f);

    if (showFps && showPing) {
        snprintf(statsBuf, sizeof(statsBuf), "%s  |  %s  |  %.0f FPS (%.1f ms)",
            appName ? appName : "PARALLAX",
            user ? user : "User",
            fps, ms);
    } else if (showFps) {
        snprintf(statsBuf, sizeof(statsBuf), "%s  |  %.0f FPS",
            appName ? appName : "PARALLAX",
            fps);
    } else {
        snprintf(statsBuf, sizeof(statsBuf), "%s  |  %s",
            appName ? appName : "PARALLAX",
            user ? user : "User");
    }

    PushFont(FontWeight::Caption);
    const ImVec2 textSize = ImGui::CalcTextSize(statsBuf);

    const float dotRadius = 3.5f * dpi;
    const float paddingX = 14.0f * dpi;
    const float paddingY = 6.0f * dpi;
    const float pillWidth = textSize.x + dotRadius * 2.0f + 10.0f * dpi + paddingX * 2.0f;
    const float pillHeight = textSize.y + paddingY * 2.0f;

    const ImVec2 pos(24.0f * dpi, 20.0f * dpi);
    const ImVec2 minPos = pos;
    const ImVec2 maxPos(pos.x + pillWidth, pos.y + pillHeight);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // Drop shadow
    drawList->AddRectFilled(
        ImVec2(minPos.x + 2.0f, minPos.y + 3.0f),
        ImVec2(maxPos.x + 2.0f, maxPos.y + 3.0f),
        IM_COL32(0, 0, 0, 90),
        pillHeight * 0.5f
    );

    // Pill background & border
    drawList->AddRectFilled(minPos, maxPos, IM_COL32(14, 16, 24, 240), pillHeight * 0.5f);
    drawList->AddRect(minPos, maxPos, ImColor(colors.Border), pillHeight * 0.5f, 0, 1.0f);

    // Glowing green online dot
    const ImVec2 dotCenter(minPos.x + paddingX + dotRadius, minPos.y + pillHeight * 0.5f);
    drawList->AddCircleFilled(dotCenter, dotRadius + 2.0f, IM_COL32(34, 197, 94, 40));
    drawList->AddCircleFilled(dotCenter, dotRadius, IM_COL32(34, 197, 94, 255));

    // Text
    const ImVec2 textPos(dotCenter.x + dotRadius + 8.0f * dpi, minPos.y + paddingY);
    drawList->AddText(textPos, ImColor(colors.TextPrimary), statsBuf);

    PopFont();
}

} // namespace parallax
