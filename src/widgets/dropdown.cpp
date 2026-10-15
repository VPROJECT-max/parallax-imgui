#include "parallax/widgets/dropdown.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <cctype>

namespace parallax {

static bool CaseInsensitiveFind(std::string_view str, std::string_view query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return it != str.end();
}

bool Dropdown(
    const char* label,
    const std::vector<std::string>& items,
    int* selected_index,
    const std::vector<std::string>& item_icons,
    float width
) {
    if (!selected_index || items.empty()) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);

    if (width <= 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }
    const float height = 30.0f * dpi;

    // Header label above dropdown if provided
    if (label && label[0] != '#' && label[1] != '#') {
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextSecondary, "%s", label);
        PopFont();
        ImGui::Spacing();
    }

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    char popupId[64];
    snprintf(popupId, sizeof(popupId), "##DDPopup_%08X", id);

    if (pressed) {
        ImGui::OpenPopup(popupId);
    }

    const bool isOpen = ImGui::IsPopupOpen(popupId);

    // Box background & border
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(hovered ? colors.CardHover : colors.CardBg), metrics.FrameRounding);
    drawList->AddRect(bb.Min, bb.Max, ImColor(isOpen ? colors.Accent : (hovered ? colors.BorderStrong : colors.Border)), metrics.FrameRounding, 0, isOpen ? 1.5f : 1.0f);

    // Current Selection Text & Icon
    std::string previewText = "Select an option...";
    std::string currentIcon;
    if (*selected_index >= 0 && *selected_index < static_cast<int>(items.size())) {
        previewText = items[*selected_index];
        if (*selected_index < static_cast<int>(item_icons.size())) {
            currentIcon = item_icons[*selected_index];
        }
    }

    float textX = bb.Min.x + 10.0f * dpi;
    if (!currentIcon.empty()) {
        const float iconDim = 14.0f * dpi;
        const ImVec2 iconPos = ImVec2(textX, bb.Min.y + (height - iconDim) * 0.5f);
        ctx.Icons().DrawIcon(drawList, currentIcon.c_str(), iconPos, ImVec2(iconDim, iconDim), colors.Accent);
        textX += iconDim + 8.0f * dpi;
    }

    const ImVec2 textPos = ImVec2(textX, bb.Min.y + (height - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(textPos, ImColor(colors.TextPrimary), previewText.c_str());

    // Chevron Icon on right edge
    const char* chevName = isOpen ? "chevron-up" : "chevron-down";
    const float chevDim = 14.0f * dpi;
    const ImVec2 chevPos = ImVec2(bb.Max.x - chevDim - 10.0f * dpi, bb.Min.y + (height - chevDim) * 0.5f);
    ctx.Icons().DrawIcon(drawList, chevName, chevPos, ImVec2(chevDim, chevDim), isOpen ? colors.Accent : colors.TextMuted);

    bool changed = false;

    // Upward opening calculation based on window bottom boundary
    const float winBottom = window->ClipRect.Max.y;
    const float screenH = ImGui::GetIO().DisplaySize.y;
    const float bottomLimit = (winBottom > 0.0f && winBottom < screenH) ? winBottom : screenH;

    const float listMaxH = 200.0f * dpi;
    const float headerExtraH = (items.size() > 5) ? (40.0f * dpi) : 0.0f;
    const float estimatedPopupH = std::min(static_cast<float>(items.size()) * 28.0f * dpi, listMaxH) + headerExtraH + 20.0f * dpi;

    ImVec2 popupPos;
    if (bb.Max.y + estimatedPopupH > bottomLimit - 12.0f * dpi && bb.Min.y - estimatedPopupH > 10.0f * dpi) {
        // Position upwards above the control
        popupPos = ImVec2(bb.Min.x, bb.Min.y - estimatedPopupH - 4.0f * dpi);
    } else {
        // Position downwards below the control
        popupPos = ImVec2(bb.Min.x, bb.Max.y + 4.0f * dpi);
    }

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi, 8.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.HeaderBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        static char s_searchQuery[64] = "";
        if (items.size() > 5) {
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputTextWithHint("##DDSearch", "Filter...", s_searchQuery, sizeof(s_searchQuery));
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        } else {
            s_searchQuery[0] = '\0';
        }

        const float maxHeight = std::min(static_cast<float>(items.size()) * 30.0f * dpi + 10.0f * dpi, listMaxH);
        ImGui::BeginChild("##DDListScroll", ImVec2(-1.0f, maxHeight), false);

        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            if (s_searchQuery[0] != '\0' && !CaseInsensitiveFind(items[i], s_searchQuery)) {
                continue;
            }

            const bool isSelected = (*selected_index == i);
            char itemHiddenId[64];
            snprintf(itemHiddenId, sizeof(itemHiddenId), "##DDItem_%d", i);

            const ImVec2 itemPos = ImGui::GetCursorScreenPos();
            const float itemH = 26.0f * dpi;

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Header, colors.AccentMuted);
                ImGui::PushStyleColor(ImGuiCol_Text, colors.Accent);
            }

            // Selectable handles click, hover, and active state
            if (ImGui::Selectable(itemHiddenId, isSelected, 0, ImVec2(0.0f, itemH))) {
                *selected_index = i;
                changed = true;
                ImGui::CloseCurrentPopup();
            }

            if (isSelected) {
                ImGui::PopStyleColor(2);
            }

            ImDrawList* listDraw = ImGui::GetWindowDrawList();
            float itemDrawX = itemPos.x + 8.0f * dpi;

            // Draw item icon first without overlapping text
            if (i < static_cast<int>(item_icons.size()) && !item_icons[i].empty()) {
                const float itemIconDim = 14.0f * dpi;
                const ImVec2 iconPos = ImVec2(itemDrawX, itemPos.y + (itemH - itemIconDim) * 0.5f);
                ctx.Icons().DrawIcon(listDraw, item_icons[i].c_str(), iconPos, ImVec2(itemIconDim, itemIconDim), isSelected ? colors.Accent : colors.TextSecondary);
                itemDrawX += itemIconDim + 8.0f * dpi;
            }

            // Draw item text clearly separated from the icon
            listDraw->AddText(
                ImVec2(itemDrawX, itemPos.y + (itemH - ImGui::GetFontSize()) * 0.5f),
                ImColor(isSelected ? colors.Accent : colors.TextPrimary),
                items[i].c_str()
            );
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return changed;
}

bool MultiDropdown(
    const char* label,
    const std::vector<std::string>& items,
    std::vector<bool>& selected_states,
    const std::vector<std::string>& item_icons,
    float width
) {
    if (items.empty()) return false;
    if (selected_states.size() < items.size()) {
        selected_states.resize(items.size(), false);
    }

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(label);

    if (width <= 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }
    const float height = 30.0f * dpi;

    if (label && label[0] != '#' && label[1] != '#') {
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextSecondary, "%s", label);
        PopFont();
        ImGui::Spacing();
    }

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    char popupId[64];
    snprintf(popupId, sizeof(popupId), "##MultiDDPopup_%08X", id);

    if (pressed) {
        ImGui::OpenPopup(popupId);
    }

    const bool isOpen = ImGui::IsPopupOpen(popupId);

    // Box background & border
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(hovered ? colors.CardHover : colors.CardBg), metrics.FrameRounding);
    drawList->AddRect(bb.Min, bb.Max, ImColor(isOpen ? colors.Accent : (hovered ? colors.BorderStrong : colors.Border)), metrics.FrameRounding, 0, isOpen ? 1.5f : 1.0f);

    // Summary representation (e.g. "None", "All", "3 Selected")
    int countSelected = 0;
    std::string firstSelectedName;
    for (size_t i = 0; i < items.size(); ++i) {
        if (selected_states[i]) {
            countSelected++;
            if (firstSelectedName.empty()) {
                firstSelectedName = items[i];
            }
        }
    }

    std::string previewText;
    if (countSelected == 0) {
        previewText = "None Selected";
    } else if (countSelected == 1) {
        previewText = firstSelectedName;
    } else if (countSelected == static_cast<int>(items.size())) {
        previewText = "All Selected";
    } else {
        previewText = std::to_string(countSelected) + " Selected";
    }

    const float textX = bb.Min.x + 10.0f * dpi;
    const ImVec2 textPos = ImVec2(textX, bb.Min.y + (height - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(textPos, ImColor(colors.TextPrimary), previewText.c_str());

    // Chevron Icon on right edge
    const char* chevName = isOpen ? "chevron-up" : "chevron-down";
    const float chevDim = 14.0f * dpi;
    const ImVec2 chevPos = ImVec2(bb.Max.x - chevDim - 10.0f * dpi, bb.Min.y + (height - chevDim) * 0.5f);
    ctx.Icons().DrawIcon(drawList, chevName, chevPos, ImVec2(chevDim, chevDim), isOpen ? colors.Accent : colors.TextMuted);

    bool anyChanged = false;

    // Upward opening calculation based on window bottom boundary
    const float winBottom = window->ClipRect.Max.y;
    const float screenH = ImGui::GetIO().DisplaySize.y;
    const float bottomLimit = (winBottom > 0.0f && winBottom < screenH) ? winBottom : screenH;

    const float listMaxH = 200.0f * dpi;
    const float headerExtraH = 38.0f * dpi; // Select All / Clear All row
    const float estimatedPopupH = std::min(static_cast<float>(items.size()) * 28.0f * dpi, listMaxH) + headerExtraH + 24.0f * dpi;

    ImVec2 popupPos;
    if (bb.Max.y + estimatedPopupH > bottomLimit - 12.0f * dpi && bb.Min.y - estimatedPopupH > 10.0f * dpi) {
        popupPos = ImVec2(bb.Min.x, bb.Min.y - estimatedPopupH - 4.0f * dpi);
    } else {
        popupPos = ImVec2(bb.Min.x, bb.Max.y + 4.0f * dpi);
    }

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi, 8.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.HeaderBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        // Styled Select All / Clear All Buttons
        ImGui::PushStyleColor(ImGuiCol_Button, colors.CardBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.CardHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors.AccentMuted);
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, metrics.FrameRounding);

        if (ImGui::Button("Select All", ImVec2(72.0f * dpi, 22.0f * dpi))) {
            std::fill(selected_states.begin(), selected_states.end(), true);
            anyChanged = true;
        }
        ImGui::SameLine(0.0f, 6.0f * dpi);
        if (ImGui::Button("Clear All", ImVec2(72.0f * dpi, 22.0f * dpi))) {
            std::fill(selected_states.begin(), selected_states.end(), false);
            anyChanged = true;
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float maxHeight = std::min(static_cast<float>(items.size()) * 28.0f * dpi, listMaxH);
        ImGui::BeginChild("##MultiDDListScroll", ImVec2(-1.0f, maxHeight), false);

        for (size_t i = 0; i < items.size(); ++i) {
            const bool state = selected_states[i];
            char rowId[64];
            snprintf(rowId, sizeof(rowId), "##MRow_%zu", i);

            const ImVec2 rowPos = ImGui::GetCursorScreenPos();
            const float itemH = 26.0f * dpi;

            // Clickable row without closing popup
            if (ImGui::Selectable(rowId, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(0.0f, itemH))) {
                selected_states[i] = !selected_states[i];
                anyChanged = true;
            }

            ImDrawList* listDraw = ImGui::GetWindowDrawList();

            // Custom Checkbox geometry
            const float boxDim = 16.0f * dpi;
            const ImVec2 boxMin = ImVec2(rowPos.x + 4.0f * dpi, rowPos.y + (itemH - boxDim) * 0.5f);
            const ImVec2 boxMax = ImVec2(boxMin.x + boxDim, boxMin.y + boxDim);

            if (state) {
                // Filled active checkbox
                listDraw->AddRectFilled(boxMin, boxMax, ImColor(colors.Accent), 3.0f * dpi);
                listDraw->AddRect(boxMin, boxMax, ImColor(colors.Accent), 3.0f * dpi, 0, 1.0f);

                // White checkmark lines
                const ImVec2 p1 = ImVec2(boxMin.x + 3.5f * dpi, boxMin.y + 8.0f * dpi);
                const ImVec2 p2 = ImVec2(boxMin.x + 6.5f * dpi, boxMin.y + 11.5f * dpi);
                const ImVec2 p3 = ImVec2(boxMin.x + 12.0f * dpi, boxMin.y + 4.5f * dpi);
                listDraw->AddLine(p1, p2, IM_COL32(255, 255, 255, 255), 2.0f * dpi);
                listDraw->AddLine(p2, p3, IM_COL32(255, 255, 255, 255), 2.0f * dpi);
            } else {
                // Empty inactive checkbox
                listDraw->AddRectFilled(boxMin, boxMax, ImColor(colors.CardBg), 3.0f * dpi);
                listDraw->AddRect(boxMin, boxMax, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);
            }

            float textDrawX = boxMax.x + 8.0f * dpi;

            // Draw item icon if present
            if (i < item_icons.size() && !item_icons[i].empty()) {
                const float itemIconDim = 14.0f * dpi;
                const ImVec2 iconPos = ImVec2(textDrawX, rowPos.y + (itemH - itemIconDim) * 0.5f);
                ctx.Icons().DrawIcon(listDraw, item_icons[i].c_str(), iconPos, ImVec2(itemIconDim, itemIconDim), state ? colors.Accent : colors.TextSecondary);
                textDrawX += itemIconDim + 6.0f * dpi;
            }

            // Draw item label
            listDraw->AddText(
                ImVec2(textDrawX, rowPos.y + (itemH - ImGui::GetFontSize()) * 0.5f),
                ImColor(state ? colors.TextPrimary : colors.TextSecondary),
                items[i].c_str()
            );
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return anyChanged;
}

} // namespace parallax
