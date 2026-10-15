#include "parallax/layout/grid.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include <algorithm>

namespace parallax::layout {

static int s_columns{1};
static int s_currentCol{0};
static float s_spacing{12.0f};
static float s_columnWidth{0.0f};

void Grid::Begin(int columns, float spacing) {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();

    s_columns = std::clamp(columns, 1, 3);
    s_currentCol = 0;
    s_spacing = spacing * dpi;

    const float totalAvail = ImGui::GetContentRegionAvail().x;
    s_columnWidth = (totalAvail - s_spacing * static_cast<float>(s_columns - 1)) / static_cast<float>(s_columns);
    s_columnWidth = std::max(s_columnWidth, 120.0f * dpi);

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
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.ChildRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f * dpi, 14.0f * dpi));

    char childId[128];
    snprintf(childId, sizeof(childId), "##GB_%s", title ? title : "card");

    bool open = ImGui::BeginChild(childId, ImVec2(w, height), true, ImGuiWindowFlags_NoScrollbar);

    if (title && title[0] != '\0') {
        if (icon && icon[0] != '\0') {
            ctx.Icons().DrawIcon(icon, ImVec2(16.0f * dpi, 16.0f * dpi), colors.Accent);
            ImGui::SameLine(0.0f, 8.0f * dpi);
        }
        ImGui::TextColored(colors.TextPrimary, "%s", title);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    return open;
}

void Groupbox::End() {
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

} // namespace parallax::layout
