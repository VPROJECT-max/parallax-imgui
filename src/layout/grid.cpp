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
    s_columns = 1;
    s_currentCol = 0;
    s_columnWidth = 0.0f;
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

    const float rounding = metrics.ChildRounding * dpi;
    const float padX = metrics.CardPadding * dpi;
    const float padY = metrics.CardPadding * dpi;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors.CardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padX, padY));

    char childId[128];
    snprintf(childId, sizeof(childId), "##GB_%s", title ? title : "card");

    ImGuiChildFlags childFlags = ImGuiChildFlags_Borders;
    float childH = 0.0f;
    if (height <= 0.0f) {
        childFlags |= ImGuiChildFlags_AutoResizeY;
    } else {
        childH = height * dpi;
    }

    // Keep groupbox child window scroll strictly locked to 0 so contents never shift relative to header
    ImGui::SetNextWindowScroll(ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    bool open = ImGui::BeginChild(childId, ImVec2(w, childH), childFlags, winFlags);

    if (title && title[0] != '\0') {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 winPos = ImGui::GetWindowPos();
        const float winW = ImGui::GetWindowWidth();
        const float headerBarH = 38.0f * dpi;

        // Elevated/tinted CardHeaderBg strip across the top of the card
        drawList->AddRectFilled(
            winPos,
            ImVec2(winPos.x + winW, winPos.y + headerBarH),
            ImColor(colors.CardHeaderBg),
            rounding,
            ImDrawFlags_RoundCornersTop
        );

        // Crisp 1px bottom border separating card header from card body
        drawList->AddLine(
            ImVec2(winPos.x, winPos.y + headerBarH),
            ImVec2(winPos.x + winW, winPos.y + headerBarH),
            ImColor(colors.BorderSubtle),
            1.0f
        );

        PushFont(FontWeight::SemiBold);
        const float fontH = ImGui::GetFontSize();
        const float iconDim = 15.0f * dpi;
        float textX = winPos.x + padX;

        if (icon && icon[0] != '\0') {
            const ImVec2 iconPos(textX, winPos.y + (headerBarH - iconDim) * 0.5f);
            ctx.Icons().DrawIcon(drawList, icon, iconPos, ImVec2(iconDim, iconDim), colors.Accent);
            textX += iconDim + 8.0f * dpi;
        }

        const char* textEnd = ImGui::FindRenderedTextEnd(title);
        const ImVec2 textPos(textX, winPos.y + (headerBarH - fontH) * 0.5f);
        drawList->AddText(nullptr, 0.0f, textPos, ImColor(colors.TextPrimary), title, textEnd);
        PopFont();

        // Advance cursor below the header strip into the padded body area without setting IsSetPos flag
        ImGuiWindow* curWin = ImGui::GetCurrentWindow();
        const ImVec2 bodyPos(winPos.x + padX, winPos.y + headerBarH + 12.0f * dpi);
        curWin->DC.CursorPos = bodyPos;
        curWin->DC.CursorMaxPos = ImMax(curWin->DC.CursorMaxPos, bodyPos);
        curWin->DC.IsSetPos = false;
    }

    return open;
}

void Groupbox::Description(const char* text) {
    if (!text || text[0] == '\0') return;
    const auto& colors = StyleManager::Get().Colors();
    PushFont(FontWeight::Caption);
    ImGui::PushStyleColor(ImGuiCol_Text, colors.TextMuted);
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
    PopFont();
    ImGui::Spacing();
}

void Groupbox::End() {
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    ImGui::Spacing();
}

} // namespace parallax::layout
