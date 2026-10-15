#include "parallax/layout/grid.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include <imgui_internal.h>
#include <algorithm>

namespace parallax::layout {

static int s_columns{1};
static int s_currentCol{0};
static float s_spacing{12.0f};
static float s_columnWidth{0.0f};

void Grid::Begin(int columns, float spacing) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();

    const float totalAvail = ImGui::GetContentRegionAvail().x;

    // Responsive reflow: if columns would be narrower than 220px, automatically reduce column count
    int effectiveCols = std::clamp(columns, 1, 3);
    if (effectiveCols == 3 && totalAvail < 660.0f * dpi) {
        effectiveCols = (totalAvail < 440.0f * dpi) ? 1 : 2;
    } else if (effectiveCols == 2 && totalAvail < 440.0f * dpi) {
        effectiveCols = 1;
    }

    s_columns = effectiveCols;
    s_currentCol = 0;
    s_spacing = spacing * dpi;

    s_columnWidth = (totalAvail - s_spacing * static_cast<float>(s_columns - 1)) / static_cast<float>(s_columns);
    s_columnWidth = std::max(s_columnWidth, 180.0f * dpi);

    ImGui::BeginGroup();
}

void Grid::NextColumn() {
    s_currentCol++;
    if (s_currentCol < s_columns) {
        ImGui::EndGroup();
        ImGui::SameLine(0.0f, s_spacing);
        ImGui::BeginGroup();
    } else {
        s_currentCol = 0;
        ImGui::EndGroup();
        ImGui::NewLine();
        ImGui::BeginGroup();
    }
}

void Grid::End() {
    ImGui::EndGroup();
}

float Grid::GetColumnWidth() noexcept {
    return s_columnWidth;
}

bool Groupbox::Begin(const char* title, const char* icon, float height) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();

    float w = Grid::GetColumnWidth();
    if (w <= 0.0f) {
        w = ImGui::GetContentRegionAvail().x;
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors.CardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.ChildRounding * dpi);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f * dpi, 16.0f * dpi));

    char childId[128];
    snprintf(childId, sizeof(childId), "##GB_%s", title ? title : "card");

    bool open = ImGui::BeginChild(childId, ImVec2(w, height), true, ImGuiWindowFlags_NoScrollbar);

    if (title && title[0] != '\0') {
        PushFont(FontWeight::SemiBold);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 startPos = ImGui::GetCursorScreenPos();
        const float fontH = ImGui::GetFontSize();
        const float iconDim = 16.0f * dpi;
        const float headerH = std::max(fontH, iconDim);
        float textX = startPos.x;

        if (icon && icon[0] != '\0') {
            const ImVec2 iconPos(textX, startPos.y + (headerH - iconDim) * 0.5f);
            ctx.Icons().DrawIcon(drawList, icon, iconPos, ImVec2(iconDim, iconDim), colors.Accent);
            textX += iconDim + 8.0f * dpi;
        }

        const char* textEnd = ImGui::FindRenderedTextEnd(title);
        const ImVec2 textPos(textX, startPos.y + (headerH - fontH) * 0.5f);
        drawList->AddText(nullptr, 0.0f, textPos, ImColor(colors.TextPrimary), title, textEnd);
        PopFont();

        // Advance cursor past header title
        ImGui::SetCursorScreenPos(ImVec2(startPos.x, startPos.y + headerH + 6.0f * dpi));

        // Custom subtle 1px divider
        const ImVec2 lineStart = ImGui::GetCursorScreenPos();
        const float lineW = ImGui::GetContentRegionAvail().x;
        drawList->AddLine(
            lineStart,
            ImVec2(lineStart.x + lineW, lineStart.y),
            ImColor(colors.Border),
            1.0f
        );

        ImGui::SetCursorScreenPos(ImVec2(lineStart.x, lineStart.y + 8.0f * dpi));
    }

    return open;
}

void Groupbox::End() {
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

} // namespace parallax::layout
