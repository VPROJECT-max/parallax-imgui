#include "parallax/widgets/color_picker.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace parallax {

static void DrawCheckerboard(ImDrawList* drawList, const ImRect& bb, float squareSize = 4.0f) {
    const ImU32 col1 = IM_COL32(50, 50, 60, 255);
    const ImU32 col2 = IM_COL32(30, 30, 40, 255);

    for (float y = bb.Min.y; y < bb.Max.y; y += squareSize) {
        for (float x = bb.Min.x; x < bb.Max.x; x += squareSize) {
            int xi = static_cast<int>((x - bb.Min.x) / squareSize);
            int yi = static_cast<int>((y - bb.Min.y) / squareSize);
            ImU32 col = ((xi + yi) % 2 == 0) ? col1 : col2;
            drawList->AddRectFilled(
                ImVec2(x, y),
                ImVec2(std::min(x + squareSize, bb.Max.x), std::min(y + squareSize, bb.Max.y)),
                col
            );
        }
    }
}

static std::string ColorToHex(const ImVec4& c, bool hasAlpha) {
    char buf[16];
    int r = std::clamp(static_cast<int>(c.x * 255.0f + 0.5f), 0, 255);
    int g = std::clamp(static_cast<int>(c.y * 255.0f + 0.5f), 0, 255);
    int b = std::clamp(static_cast<int>(c.z * 255.0f + 0.5f), 0, 255);
    int a = std::clamp(static_cast<int>(c.w * 255.0f + 0.5f), 0, 255);

    if (hasAlpha) {
        snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", r, g, b, a);
    } else {
        snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
    }
    return std::string(buf);
}

static bool HexToColor(const char* hex, ImVec4& outCol, bool hasAlpha) {
    if (!hex || hex[0] == '\0') return false;
    if (hex[0] == '#') hex++;

    size_t len = strlen(hex);
    if (len != 6 && len != 8) return false;

    unsigned int r = 0, g = 0, b = 0, a = 255;
    if (len == 6) {
        if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) != 3) return false;
    } else if (len == 8) {
        if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) != 4) return false;
    }

    outCol.x = static_cast<float>(r) / 255.0f;
    outCol.y = static_cast<float>(g) / 255.0f;
    outCol.z = static_cast<float>(b) / 255.0f;
    if (hasAlpha) {
        outCol.w = static_cast<float>(a) / 255.0f;
    }
    return true;
}

bool ColorPicker(const char* str_id, ImVec4* color, bool hasAlpha, const ImVec2& size) {
    if (!color) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(str_id);

    ImVec2 swatchSize = size;
    if (swatchSize.x <= 0.0f) swatchSize.x = 22.0f * dpi;
    if (swatchSize.y <= 0.0f) swatchSize.y = 22.0f * dpi;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + swatchSize.x, pos.y + swatchSize.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Context menu on right click
    char ctxMenuId[64];
    snprintf(ctxMenuId, sizeof(ctxMenuId), "##CPContextMenu_%s", str_id);
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(ctxMenuId);
    }

    // Swatch Rendering
    const float rounding = 4.0f * dpi;
    if (hasAlpha && color->w < 0.99f) {
        DrawCheckerboard(drawList, bb, 4.0f * dpi);
    }
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(*color), rounding);

    // Border with hover effect
    ImVec4 borderCol = hovered ? colors.BorderStrong : colors.Border;
    drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), rounding, 0, 1.0f);

    // Popup ID
    char popupId[64];
    snprintf(popupId, sizeof(popupId), "##CPPopup_%s", str_id);

    if (pressed) {
        ImGui::OpenPopup(popupId);
    }

    bool changed = false;

    // Right-click Quick Copy/Paste Menu
    if (ImGui::BeginPopup(ctxMenuId)) {
        if (ImGui::MenuItem("Copy HEX")) {
            ImGui::SetClipboardText(ColorToHex(*color, hasAlpha).c_str());
        }
        if (ImGui::MenuItem("Paste HEX")) {
            const char* clip = ImGui::GetClipboardText();
            if (HexToColor(clip, *color, hasAlpha)) {
                changed = true;
            }
        }
        ImGui::EndPopup();
    }

    // Color Picker Popup
    ImGui::SetNextWindowSize(ImVec2(240.0f * dpi, 0.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f * dpi, 12.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.HeaderBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId)) {
        ImGui::TextColored(colors.TextPrimary, "Color Palette");
        ImGui::Spacing();

        ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueBar |
                                    ImGuiColorEditFlags_NoSidePreview |
                                    ImGuiColorEditFlags_DisplayRGB |
                                    ImGuiColorEditFlags_DisplayHex |
                                    ImGuiColorEditFlags_InputRGB;
        if (hasAlpha) {
            flags |= ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf;
        }

        float colArr[4] = { color->x, color->y, color->z, color->w };
        if (ImGui::ColorPicker4("##ParallaxColorPickerInternal", colArr, flags)) {
            color->x = colArr[0];
            color->y = colArr[1];
            color->z = colArr[2];
            color->w = colArr[3];
            changed = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Preset Swatches
        ImGui::TextColored(colors.TextMuted, "Saved Swatches:");
        const ImVec4 presets[] = {
            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),      // White
            ImVec4(0.1f, 0.12f, 0.16f, 1.0f),    // Slate
            ImVec4(0.93f, 0.27f, 0.27f, 1.0f),   // Crimson
            ImVec4(0.96f, 0.62f, 0.04f, 1.0f),   // Amber
            ImVec4(0.13f, 0.77f, 0.37f, 1.0f),   // Emerald
            ImVec4(0.02f, 0.71f, 0.83f, 1.0f),   // Cyan
            ImVec4(0.39f, 0.40f, 0.95f, 1.0f),   // Indigo (Accent)
            ImVec4(0.66f, 0.33f, 0.98f, 1.0f),   // Purple
        };

        for (size_t p = 0; p < sizeof(presets) / sizeof(presets[0]); ++p) {
            if (p > 0) ImGui::SameLine(0.0f, 6.0f * dpi);

            char swatchBtnId[32];
            snprintf(swatchBtnId, sizeof(swatchBtnId), "##Preset_%zu", p);

            ImGui::PushStyleColor(ImGuiCol_Button, presets[p]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, presets[p]);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, presets[p]);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f * dpi);

            if (ImGui::Button(swatchBtnId, ImVec2(18.0f * dpi, 18.0f * dpi))) {
                color->x = presets[p].x;
                color->y = presets[p].y;
                color->z = presets[p].z;
                if (!hasAlpha) color->w = 1.0f;
                changed = true;
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
        }

        ImGui::Spacing();

        // Copy / Paste row
        if (ImGui::Button("Copy", ImVec2(100.0f * dpi, 22.0f * dpi))) {
            ImGui::SetClipboardText(ColorToHex(*color, hasAlpha).c_str());
        }
        ImGui::SameLine(0.0f, 8.0f * dpi);
        if (ImGui::Button("Paste", ImVec2(100.0f * dpi, 22.0f * dpi))) {
            const char* clip = ImGui::GetClipboardText();
            if (HexToColor(clip, *color, hasAlpha)) {
                changed = true;
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return changed;
}

bool ColorPicker(const char* str_id, float* col3or4, bool hasAlpha, const ImVec2& size) {
    if (!col3or4) return false;
    ImVec4 col(col3or4[0], col3or4[1], col3or4[2], hasAlpha ? col3or4[3] : 1.0f);
    if (ColorPicker(str_id, &col, hasAlpha, size)) {
        col3or4[0] = col.x;
        col3or4[1] = col.y;
        col3or4[2] = col.z;
        if (hasAlpha) col3or4[3] = col.w;
        return true;
    }
    return false;
}

} // namespace parallax
