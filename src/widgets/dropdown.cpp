#include "parallax/widgets/dropdown.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
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
        ImGui::TextColored(colors.TextSecondary, "%s", label);
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
        auto sub = ctx.Icons().GetIcon(currentIcon);
        if (sub.valid && sub.srv) {
            const float iconDim = 14.0f * dpi;
            const ImVec2 iconPos = ImVec2(textX, bb.Min.y + (height - iconDim) * 0.5f);
            drawList->AddImage(
                reinterpret_cast<ImTextureID>(sub.srv),
                iconPos,
                ImVec2(iconPos.x + iconDim, iconPos.y + iconDim),
                sub.uv0, sub.uv1,
                ImColor(colors.Accent)
            );
            textX += iconDim + 8.0f * dpi;
        }
    }

    const ImVec2 textPos = ImVec2(textX, bb.Min.y + (height - ImGui::GetFontSize()) * 0.5f);
    drawList->AddText(textPos, ImColor(colors.TextPrimary), previewText.c_str());

    // Chevron Icon on right edge
    auto chevronSub = ctx.Icons().GetIcon("chevron-down");
    if (chevronSub.valid && chevronSub.srv) {
        const float chevDim = 14.0f * dpi;
        const ImVec2 chevPos = ImVec2(bb.Max.x - chevDim - 10.0f * dpi, bb.Min.y + (height - chevDim) * 0.5f);
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(chevronSub.srv),
            chevPos,
            ImVec2(chevPos.x + chevDim, chevPos.y + chevDim),
            chevronSub.uv0, chevronSub.uv1,
            ImColor(isOpen ? colors.Accent : colors.TextMuted)
        );
    }

    bool changed = false;

    // Dropdown list popup
    ImGui::SetNextWindowPos(ImVec2(bb.Min.x, bb.Max.y + 4.0f * dpi));
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi, 8.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.HeaderBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        // Search filter for longer lists
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

        const float maxHeight = std::min(static_cast<float>(items.size()) * 30.0f * dpi + 20.0f * dpi, 200.0f * dpi);
        ImGui::BeginChild("##DDListScroll", ImVec2(-1.0f, maxHeight), false);

        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            if (s_searchQuery[0] != '\0' && !CaseInsensitiveFind(items[i], s_searchQuery)) {
                continue;
            }

            const bool isSelected = (*selected_index == i);
            char itemStrId[128];
            snprintf(itemStrId, sizeof(itemStrId), "%s##DDItem_%d", items[i].c_str(), i);

            ImVec2 itemPos = ImGui::GetCursorScreenPos();
            const float itemH = 26.0f * dpi;

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Header, colors.AccentMuted);
                ImGui::PushStyleColor(ImGuiCol_Text, colors.Accent);
            }

            if (ImGui::Selectable(itemStrId, isSelected, 0, ImVec2(0.0f, itemH))) {
                *selected_index = i;
                changed = true;
                ImGui::CloseCurrentPopup();
            }

            if (isSelected) {
                ImGui::PopStyleColor(2);
            }

            // Draw item icon if available
            if (i < static_cast<int>(item_icons.size()) && !item_icons[i].empty()) {
                auto itemSub = ctx.Icons().GetIcon(item_icons[i]);
                if (itemSub.valid && itemSub.srv) {
                    ImDrawList* listDraw = ImGui::GetWindowDrawList();
                    const float itemIconDim = 14.0f * dpi;
                    listDraw->AddImage(
                        reinterpret_cast<ImTextureID>(itemSub.srv),
                        ImVec2(itemPos.x + 4.0f * dpi, itemPos.y + (itemH - itemIconDim) * 0.5f),
                        ImVec2(itemPos.x + 4.0f * dpi + itemIconDim, itemPos.y + (itemH + itemIconDim) * 0.5f),
                        itemSub.uv0, itemSub.uv1,
                        ImColor(isSelected ? colors.Accent : colors.TextSecondary)
                    );
                }
            }
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
        ImGui::TextColored(colors.TextSecondary, "%s", label);
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
    auto chevronSub = ctx.Icons().GetIcon("chevron-down");
    if (chevronSub.valid && chevronSub.srv) {
        const float chevDim = 14.0f * dpi;
        const ImVec2 chevPos = ImVec2(bb.Max.x - chevDim - 10.0f * dpi, bb.Min.y + (height - chevDim) * 0.5f);
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(chevronSub.srv),
            chevPos,
            ImVec2(chevPos.x + chevDim, chevPos.y + chevDim),
            chevronSub.uv0, chevronSub.uv1,
            ImColor(isOpen ? colors.Accent : colors.TextMuted)
        );
    }

    bool anyChanged = false;

    ImGui::SetNextWindowPos(ImVec2(bb.Min.x, bb.Max.y + 4.0f * dpi));
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi, 8.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.HeaderBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        // Select All / Clear All Row
        if (ImGui::SmallButton("Select All")) {
            std::fill(selected_states.begin(), selected_states.end(), true);
            anyChanged = true;
        }
        ImGui::SameLine(0.0f, 8.0f * dpi);
        if (ImGui::SmallButton("Clear All")) {
            std::fill(selected_states.begin(), selected_states.end(), false);
            anyChanged = true;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float maxHeight = std::min(static_cast<float>(items.size()) * 28.0f * dpi + 20.0f * dpi, 200.0f * dpi);
        ImGui::BeginChild("##MultiDDListScroll", ImVec2(-1.0f, maxHeight), false);

        for (size_t i = 0; i < items.size(); ++i) {
            bool state = selected_states[i];
            char checkLabel[128];
            snprintf(checkLabel, sizeof(checkLabel), "%s##MDDItem_%zu", items[i].c_str(), i);

            if (i < item_icons.size() && !item_icons[i].empty()) {
                ctx.Icons().DrawIcon(item_icons[i].c_str(), ImVec2(14.0f * dpi, 14.0f * dpi), state ? colors.Accent : colors.TextMuted);
                ImGui::SameLine(0.0f, 6.0f * dpi);
            }

            if (ImGui::Checkbox(checkLabel, &state)) {
                selected_states[i] = state;
                anyChanged = true;
            }
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return anyChanged;
}

} // namespace parallax
