#include "parallax/widgets/dropdown.hpp"
#include "parallax/widgets/checkbox.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace parallax {

namespace {

bool CaseInsensitiveFind(std::string_view str, std::string_view query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) {
            return std::tolower(static_cast<unsigned char>(ch1)) == std::tolower(static_cast<unsigned char>(ch2));
        }
    );
    return it != str.end();
}

void DrawChevronVector(ImDrawList* drawList, const ImVec2& center, float halfSpan, float openProgress, ImU32 col, float dpi) {
    // Smoothly morphs between downward chevron (openProgress=0) and upward chevron (openProgress=1)
    const float dir = 1.0f - 2.0f * std::clamp(openProgress, 0.0f, 1.0f);
    const float dy = halfSpan * 0.55f * dir;
    const ImVec2 p1(center.x - halfSpan, center.y - dy);
    const ImVec2 p2(center.x,            center.y + dy);
    const ImVec2 p3(center.x + halfSpan, center.y - dy);
    drawList->AddLine(p1, p2, col, 1.6f * dpi);
    drawList->AddLine(p2, p3, col, 1.6f * dpi);
}

} // namespace

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
    const float height = metrics.ControlHeight * dpi;

    // Header label above dropdown if provided
    if (label && !(label[0] == '#' && label[1] == '#')) {
        PushFont(FontWeight::Medium);
        const char* labelEnd = ImGui::FindRenderedTextEnd(label);
        if (labelEnd > label) {
            ImGui::TextColored(colors.TextSecondary, "%.*s", static_cast<int>(labelEnd - label), label);
            ImGui::Dummy(ImVec2(0.0f, 1.0f * dpi));
        }
        PopFont();
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
    const float openAnim = ctx.Anim().SpringFloat(id + 0x4100, isOpen ? 1.0f : 0.0f, {240.0f, 24.0f, 1.0f});
    const float hoverAnim = ctx.Anim().AnimFloat(id + 0x4200, (hovered || isOpen) ? 1.0f : 0.0f, metrics.AnimFast);

    // Distinct Interactive Control Surface
    const ImVec4 triggerBg(
        colors.WidgetBg.x + (colors.WidgetHover.x - colors.WidgetBg.x) * hoverAnim,
        colors.WidgetBg.y + (colors.WidgetHover.y - colors.WidgetBg.y) * hoverAnim,
        colors.WidgetBg.z + (colors.WidgetHover.z - colors.WidgetBg.z) * hoverAnim,
        1.0f
    );
    const ImVec4 triggerBorder = isOpen ? colors.Accent : (hovered ? colors.BorderStrong : colors.Border);

    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(triggerBg), metrics.FrameRounding * dpi);
    drawList->AddRect(bb.Min, bb.Max, ImColor(triggerBorder), metrics.FrameRounding * dpi, 0, isOpen ? 1.5f : 1.0f);

    // Left accent status indicator bar when open
    if (openAnim > 0.02f) {
        const float barH = (height - 12.0f * dpi) * openAnim;
        const float midY = (bb.Min.y + bb.Max.y) * 0.5f;
        drawList->AddRectFilled(
            ImVec2(bb.Min.x + 3.0f * dpi, midY - barH * 0.5f),
            ImVec2(bb.Min.x + 5.5f * dpi, midY + barH * 0.5f),
            ImColor(colors.Accent),
            1.5f * dpi
        );
    }

    // Current Selection Text & Icon
    std::string previewText = "Select an option...";
    std::string currentIcon;
    const bool hasValidSelection = (*selected_index >= 0 && *selected_index < static_cast<int>(items.size()));
    if (hasValidSelection) {
        previewText = items[*selected_index];
        if (*selected_index < static_cast<int>(item_icons.size())) {
            currentIcon = item_icons[*selected_index];
        }
    }

    float textX = bb.Min.x + (10.0f + 3.0f * openAnim) * dpi;
    if (!currentIcon.empty()) {
        const float iconDim = 14.0f * dpi;
        const ImVec2 iconPos(textX, bb.Min.y + (height - iconDim) * 0.5f);
        ctx.Icons().DrawIcon(drawList, currentIcon.c_str(), iconPos, ImVec2(iconDim, iconDim), colors.Accent);
        textX += iconDim + 8.0f * dpi;
    }

    const float rightPad = 28.0f * dpi;
    drawList->PushClipRect(ImVec2(textX, bb.Min.y), ImVec2(bb.Max.x - rightPad, bb.Max.y), true);
    PushFont(FontWeight::Medium);
    const ImVec2 textPos(textX, bb.Min.y + (height - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(textPos, ImColor(hasValidSelection ? colors.TextPrimary : colors.TextMuted), previewText.c_str());
    PopFont();
    drawList->PopClipRect();

    // Animated vector chevron on right edge
    const ImVec2 chevCenter(bb.Max.x - 14.0f * dpi, (bb.Min.y + bb.Max.y) * 0.5f);
    DrawChevronVector(drawList, chevCenter, 4.2f * dpi, openAnim, ImColor(isOpen ? colors.Accent : colors.TextSecondary), dpi);

    bool changed = false;

    const float winBottom = window->ClipRect.Max.y;
    const float screenH = ImGui::GetIO().DisplaySize.y;
    const float bottomLimit = (winBottom > 0.0f && winBottom < screenH) ? winBottom : screenH;

    const bool showSearch = (items.size() >= 5);
    const float listMaxH = 196.0f * dpi;
    const float headerExtraH = showSearch ? (36.0f * dpi) : 0.0f;
    const float estimatedPopupH = std::min(static_cast<float>(items.size()) * 28.0f * dpi, listMaxH) + headerExtraH + 16.0f * dpi;

    ImVec2 popupPos;
    if (bb.Max.y + estimatedPopupH > bottomLimit - 10.0f * dpi && bb.Min.y - estimatedPopupH > 10.0f * dpi) {
        popupPos = ImVec2(bb.Min.x, bb.Min.y - estimatedPopupH - 4.0f * dpi);
    } else {
        popupPos = ImVec2(bb.Min.x, bb.Max.y + 4.0f * dpi);
    }

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f * dpi, 6.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding * dpi);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.PopupBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        static std::unordered_map<ImGuiID, std::string> s_searchQueries;
        std::string& searchStr = s_searchQueries[id];
        if (ImGui::IsWindowAppearing()) {
            searchStr.clear();
        }

        if (showSearch) {
            char searchBuf[64]{};
            snprintf(searchBuf, sizeof(searchBuf), "%s", searchStr.c_str());
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.WidgetBg);
            if (ImGui::InputTextWithHint("##DDSearch", "Filter options...", searchBuf, sizeof(searchBuf))) {
                searchStr = searchBuf;
            }
            ImGui::PopStyleColor();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        const float maxHeight = std::min(static_cast<float>(items.size()) * 29.0f * dpi + 4.0f * dpi, listMaxH);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::BeginChild("##DDListScroll", ImVec2(-1.0f, maxHeight), false);

        int visibleCount = 0;
        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            if (!searchStr.empty() && !CaseInsensitiveFind(items[i], searchStr)) {
                continue;
            }
            visibleCount++;

            const bool isSelected = (*selected_index == i);
            char itemHiddenId[64];
            snprintf(itemHiddenId, sizeof(itemHiddenId), "##DDItem_%d", i);

            const ImVec2 itemPos = ImGui::GetCursorScreenPos();
            const float itemW = ImGui::GetContentRegionAvail().x;
            const float itemH = 26.0f * dpi;

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));

            if (ImGui::Selectable(itemHiddenId, isSelected, 0, ImVec2(itemW, itemH))) {
                *selected_index = i;
                changed = true;
                ImGui::CloseCurrentPopup();
            }
            const bool itemHovered = ImGui::IsItemHovered();

            ImGui::PopStyleColor(3);

            ImDrawList* listDraw = ImGui::GetWindowDrawList();
            const ImVec2 rowMax(itemPos.x + itemW, itemPos.y + itemH);

            if (isSelected || itemHovered) {
                ImVec4 rowBg = isSelected ? colors.AccentMuted : colors.WidgetHover;
                if (isSelected && itemHovered) {
                    rowBg.w = std::min(1.0f, rowBg.w + 0.08f);
                }
                listDraw->AddRectFilled(itemPos, rowMax, ImColor(rowBg), 5.0f * dpi);
            }

            if (isSelected) {
                listDraw->AddRectFilled(
                    ImVec2(itemPos.x + 2.0f * dpi, itemPos.y + 5.0f * dpi),
                    ImVec2(itemPos.x + 4.5f * dpi, rowMax.y - 5.0f * dpi),
                    ImColor(colors.Accent),
                    1.5f * dpi
                );
            }

            float itemDrawX = itemPos.x + (isSelected ? 10.0f : 8.0f) * dpi;

            if (i < static_cast<int>(item_icons.size()) && !item_icons[i].empty()) {
                const float itemIconDim = 14.0f * dpi;
                const ImVec2 iconPos(itemDrawX, itemPos.y + (itemH - itemIconDim) * 0.5f);
                ctx.Icons().DrawIcon(
                    listDraw,
                    item_icons[i].c_str(),
                    iconPos,
                    ImVec2(itemIconDim, itemIconDim),
                    isSelected ? colors.Accent : colors.TextSecondary
                );
                itemDrawX += itemIconDim + 8.0f * dpi;
            }

            PushFont(isSelected ? FontWeight::Medium : FontWeight::Regular);
            listDraw->AddText(
                ImVec2(itemDrawX, itemPos.y + (itemH - ImGui::GetFontSize()) * 0.5f),
                ImColor(isSelected ? colors.Accent : (itemHovered ? colors.TextPrimary : colors.TextSecondary)),
                items[i].c_str()
            );
            PopFont();

            if (isSelected) {
                const float cx = rowMax.x - 14.0f * dpi;
                const float cy = itemPos.y + itemH * 0.5f;
                listDraw->AddLine(ImVec2(cx - 3.5f * dpi, cy + 0.5f * dpi), ImVec2(cx - 1.0f * dpi, cy + 3.0f * dpi), ImColor(colors.Accent), 1.6f * dpi);
                listDraw->AddLine(ImVec2(cx - 1.0f * dpi, cy + 3.0f * dpi), ImVec2(cx + 4.0f * dpi, cy - 2.5f * dpi), ImColor(colors.Accent), 1.6f * dpi);
            }
        }

        if (visibleCount == 0) {
            ImGui::TextColored(colors.TextMuted, "No matching options");
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
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
    const float height = metrics.ControlHeight * dpi;

    if (label && !(label[0] == '#' && label[1] == '#')) {
        PushFont(FontWeight::Medium);
        const char* labelEnd = ImGui::FindRenderedTextEnd(label);
        if (labelEnd > label) {
            ImGui::TextColored(colors.TextSecondary, "%.*s", static_cast<int>(labelEnd - label), label);
            ImGui::Dummy(ImVec2(0.0f, 1.0f * dpi));
        }
        PopFont();
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
    const float openAnim = ctx.Anim().SpringFloat(id + 0x5100, isOpen ? 1.0f : 0.0f, {240.0f, 24.0f, 1.0f});
    const float hoverAnim = ctx.Anim().AnimFloat(id + 0x5200, (hovered || isOpen) ? 1.0f : 0.0f, metrics.AnimFast);

    const ImVec4 triggerBg(
        colors.WidgetBg.x + (colors.WidgetHover.x - colors.WidgetBg.x) * hoverAnim,
        colors.WidgetBg.y + (colors.WidgetHover.y - colors.WidgetBg.y) * hoverAnim,
        colors.WidgetBg.z + (colors.WidgetHover.z - colors.WidgetBg.z) * hoverAnim,
        1.0f
    );
    const ImVec4 triggerBorder = isOpen ? colors.Accent : (hovered ? colors.BorderStrong : colors.Border);

    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(triggerBg), metrics.FrameRounding * dpi);
    drawList->AddRect(bb.Min, bb.Max, ImColor(triggerBorder), metrics.FrameRounding * dpi, 0, isOpen ? 1.5f : 1.0f);

    // Summary representation + count badge
    int countSelected = 0;
    std::string summaryList;
    for (size_t i = 0; i < items.size(); ++i) {
        if (selected_states[i]) {
            countSelected++;
            if (!summaryList.empty()) {
                summaryList += ", ";
            }
            summaryList += items[i];
        }
    }

    std::string previewText = (countSelected == 0) ? "None selected" : summaryList;

    // Count pill on right side next to chevron when > 0 selected
    float rightOccupied = 28.0f * dpi;
    if (countSelected > 0) {
        char countBuf[16];
        snprintf(countBuf, sizeof(countBuf), "%d", countSelected);
        PushFont(FontWeight::Caption);
        const ImVec2 countSz = ImGui::CalcTextSize(countBuf);
        const float pillW = std::max(18.0f * dpi, countSz.x + 10.0f * dpi);
        const float pillH = 17.0f * dpi;
        const ImVec2 pillMin(bb.Max.x - 26.0f * dpi - pillW, bb.Min.y + (height - pillH) * 0.5f);
        const ImVec2 pillMax(pillMin.x + pillW, pillMin.y + pillH);
        drawList->AddRectFilled(pillMin, pillMax, ImColor(colors.AccentMuted), pillH * 0.5f);
        drawList->AddRect(pillMin, pillMax, ImColor(colors.Accent), pillH * 0.5f, 0, 1.0f);
        drawList->AddText(
            ImVec2(pillMin.x + (pillW - countSz.x) * 0.5f, pillMin.y + (pillH - countSz.y) * 0.5f),
            ImColor(colors.Accent),
            countBuf
        );
        PopFont();
        rightOccupied += pillW + 6.0f * dpi;
    }

    const float textX = bb.Min.x + 10.0f * dpi;
    drawList->PushClipRect(ImVec2(textX, bb.Min.y), ImVec2(bb.Max.x - rightOccupied, bb.Max.y), true);
    PushFont(FontWeight::Medium);
    const ImVec2 textPos(textX, bb.Min.y + (height - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(textPos, ImColor(countSelected > 0 ? colors.TextPrimary : colors.TextMuted), previewText.c_str());
    PopFont();
    drawList->PopClipRect();

    // Animated chevron
    const ImVec2 chevCenter(bb.Max.x - 14.0f * dpi, (bb.Min.y + bb.Max.y) * 0.5f);
    DrawChevronVector(drawList, chevCenter, 4.2f * dpi, openAnim, ImColor(isOpen ? colors.Accent : colors.TextSecondary), dpi);

    bool anyChanged = false;

    const float winBottom = window->ClipRect.Max.y;
    const float screenH = ImGui::GetIO().DisplaySize.y;
    const float bottomLimit = (winBottom > 0.0f && winBottom < screenH) ? winBottom : screenH;

    const float listMaxH = 196.0f * dpi;
    const float headerExtraH = 38.0f * dpi;
    const float estimatedPopupH = std::min(static_cast<float>(items.size()) * 28.0f * dpi, listMaxH) + headerExtraH + 20.0f * dpi;

    ImVec2 popupPos;
    if (bb.Max.y + estimatedPopupH > bottomLimit - 10.0f * dpi && bb.Min.y - estimatedPopupH > 10.0f * dpi) {
        popupPos = ImVec2(bb.Min.x, bb.Min.y - estimatedPopupH - 4.0f * dpi);
    } else {
        popupPos = ImVec2(bb.Min.x, bb.Max.y + 4.0f * dpi);
    }

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f * dpi, 6.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding * dpi);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.PopupBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        // Select All / Clear All Action Row
        ImGui::PushStyleColor(ImGuiCol_Button, colors.WidgetBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.WidgetHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors.AccentMuted);
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * dpi);

        const float actionW = (ImGui::GetContentRegionAvail().x - 6.0f * dpi) * 0.5f;
        if (ImGui::Button("Select All", ImVec2(actionW, 22.0f * dpi))) {
            std::fill(selected_states.begin(), selected_states.end(), true);
            anyChanged = true;
        }
        ImGui::SameLine(0.0f, 6.0f * dpi);
        if (ImGui::Button("Clear All", ImVec2(actionW, 22.0f * dpi))) {
            std::fill(selected_states.begin(), selected_states.end(), false);
            anyChanged = true;
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float maxHeight = std::min(static_cast<float>(items.size()) * 29.0f * dpi + 4.0f * dpi, listMaxH);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::BeginChild("##MultiDDListScroll", ImVec2(-1.0f, maxHeight), false);

        for (size_t i = 0; i < items.size(); ++i) {
            char rowId[64];
            snprintf(rowId, sizeof(rowId), "##MRow_%zu", i);

            const ImVec2 rowPos = ImGui::GetCursorScreenPos();
            const float itemW = ImGui::GetContentRegionAvail().x;
            const float itemH = 26.0f * dpi;

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));

            if (ImGui::Selectable(rowId, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(itemW, itemH))) {
                selected_states[i] = !selected_states[i];
                anyChanged = true;
            }
            const bool rowHovered = ImGui::IsItemHovered();
            ImGui::PopStyleColor(3);

            const bool state = selected_states[i];
            ImDrawList* listDraw = ImGui::GetWindowDrawList();

            if (rowHovered || state) {
                ImVec4 rowBg = state ? colors.AccentMuted : colors.WidgetHover;
                if (state && !rowHovered) {
                    rowBg.w *= 0.55f;
                }
                listDraw->AddRectFilled(rowPos, ImVec2(rowPos.x + itemW, rowPos.y + itemH), ImColor(rowBg), 5.0f * dpi);
            }

            // Custom Parallax Checkbox inside multi-dropdown row
            const float boxDim = 15.0f * dpi;
            const ImVec2 boxMin(rowPos.x + 6.0f * dpi, rowPos.y + (itemH - boxDim) * 0.5f);
            const ImVec2 boxMax(boxMin.x + boxDim, boxMin.y + boxDim);
            DrawParallaxCheckboxGlyph(listDraw, boxMin, boxMax, state ? 1.0f : 0.0f, rowHovered ? 1.0f : 0.0f, false, false);

            float textDrawX = boxMax.x + 8.0f * dpi;

            if (i < item_icons.size() && !item_icons[i].empty()) {
                const float itemIconDim = 14.0f * dpi;
                const ImVec2 iconPos(textDrawX, rowPos.y + (itemH - itemIconDim) * 0.5f);
                ctx.Icons().DrawIcon(
                    listDraw,
                    item_icons[i].c_str(),
                    iconPos,
                    ImVec2(itemIconDim, itemIconDim),
                    state ? colors.Accent : colors.TextSecondary
                );
                textDrawX += itemIconDim + 7.0f * dpi;
            }

            PushFont(state ? FontWeight::Medium : FontWeight::Regular);
            listDraw->AddText(
                ImVec2(textDrawX, rowPos.y + (itemH - ImGui::GetFontSize()) * 0.5f),
                ImColor(state ? colors.TextPrimary : (rowHovered ? colors.TextPrimary : colors.TextSecondary)),
                items[i].c_str()
            );
            PopFont();
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return anyChanged;
}

} // namespace parallax
