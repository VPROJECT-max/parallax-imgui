#include "parallax/widgets/color_picker.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/anim/manager.hpp"
#include <imgui_internal.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <unordered_map>

namespace parallax {

static void DrawCheckerboard(ImDrawList* drawList, const ImRect& bb, float squareSize = 4.0f) {
    const bool light = StyleManager::Get().IsLightTheme();
    const ImU32 col1 = light ? IM_COL32(225, 223, 218, 255) : IM_COL32(54, 58, 68, 255);
    const ImU32 col2 = light ? IM_COL32(198, 195, 188, 255) : IM_COL32(34, 37, 45, 255);

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
    while (*hex == ' ' || *hex == '\t') hex++;
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

static std::unordered_map<ImGuiID, float> s_hueCache;
static std::unordered_map<ImGuiID, ImVec4> s_openOriginColor;

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
    if (swatchSize.x <= 0.0f) swatchSize.x = 24.0f * dpi;
    if (swatchSize.y <= 0.0f) swatchSize.y = 20.0f * dpi;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + swatchSize.x, pos.y + swatchSize.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    char ctxMenuId[64];
    snprintf(ctxMenuId, sizeof(ctxMenuId), "##CPContextMenu_%08X", id);
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(ctxMenuId);
    }

    char popupId[64];
    snprintf(popupId, sizeof(popupId), "##CPPopup_%08X", id);

    if (pressed) {
        s_openOriginColor[id] = *color;
        ImGui::OpenPopup(popupId);
    }

    const bool isOpen = ImGui::IsPopupOpen(popupId);

    // Framed hardware color well swatch
    const float outerRounding = 5.0f * dpi;
    const float innerPad = 2.0f * dpi;
    const ImRect innerBb(
        ImVec2(bb.Min.x + innerPad, bb.Min.y + innerPad),
        ImVec2(bb.Max.x - innerPad, bb.Max.y - innerPad)
    );

    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(hovered ? colors.WidgetHover : colors.WidgetBg), outerRounding);
    if (hasAlpha && color->w < 0.99f) {
        DrawCheckerboard(drawList, innerBb, 3.5f * dpi);
    }
    drawList->AddRectFilled(innerBb.Min, innerBb.Max, ImColor(*color), std::max(2.5f * dpi, outerRounding - innerPad));

    const ImVec4 borderCol = isOpen ? colors.Accent : (hovered ? colors.BorderStrong : colors.Border);
    drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), outerRounding, 0, isOpen ? 1.5f : 1.0f);

    bool changed = false;

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

    const float popupW = 248.0f * dpi;
    const float popupH = hasAlpha ? (338.0f * dpi) : (312.0f * dpi);

    const float winBottom = window->ClipRect.Max.y;
    const float screenH = ImGui::GetIO().DisplaySize.y;
    const float bottomLimit = (winBottom > 0.0f && winBottom < screenH) ? winBottom : screenH;

    ImVec2 popupPos;
    if (bb.Max.y + popupH > bottomLimit - 12.0f * dpi && bb.Min.y - popupH > 10.0f * dpi) {
        popupPos = ImVec2(std::max(10.0f * dpi, bb.Max.x - popupW), bb.Min.y - popupH - 4.0f * dpi);
    } else {
        popupPos = ImVec2(std::max(10.0f * dpi, bb.Max.x - popupW), bb.Max.y + 4.0f * dpi);
    }

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(popupW, 0.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f * dpi, 12.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding * dpi);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.PopupBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId)) {
        ImDrawList* popDraw = ImGui::GetWindowDrawList();
        const float canvasW = popupW - 24.0f * dpi;

        // Header Row: Title on left, Original vs Current live comparison swatch on right
        const ImVec2 headerPos = ImGui::GetCursorScreenPos();
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextPrimary, "Color Inspector");
        PopFont();

        const ImVec4 origCol = (s_openOriginColor.find(id) != s_openOriginColor.end()) ? s_openOriginColor[id] : *color;
        const float compW = 48.0f * dpi;
        const float compH = 18.0f * dpi;
        const ImVec2 compMin(headerPos.x + canvasW - compW, headerPos.y);
        const ImVec2 compMax(compMin.x + compW, compMin.y + compH);
        const ImVec2 compMid(compMin.x + compW * 0.5f, compMax.y);

        if (hasAlpha && (origCol.w < 0.99f || color->w < 0.99f)) {
            DrawCheckerboard(popDraw, ImRect(compMin, compMax), 3.0f * dpi);
        }
        popDraw->AddRectFilled(compMin, compMid, ImColor(origCol), 4.0f * dpi, ImDrawFlags_RoundCornersLeft);
        popDraw->AddRectFilled(ImVec2(compMid.x, compMin.y), compMax, ImColor(*color), 4.0f * dpi, ImDrawFlags_RoundCornersRight);
        popDraw->AddRect(compMin, compMax, ImColor(colors.BorderStrong), 4.0f * dpi, 0, 1.0f);

        // Clicking left half of comparison swatch reverts to original color
        const ImRect revertBb(compMin, compMid);
        if (revertBb.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseClicked(0)) {
            *color = origCol;
            changed = true;
        }

        ImGui::Spacing();

        // Convert current color to HSV
        float H = 0.0f, S = 0.0f, V = 0.0f;
        ImGui::ColorConvertRGBtoHSV(color->x, color->y, color->z, H, S, V);

        if (s_hueCache.find(id) == s_hueCache.end()) {
            s_hueCache[id] = H;
        } else if (S > 0.02f && V > 0.02f) {
            s_hueCache[id] = H;
        } else {
            H = s_hueCache[id];
        }

        const float svH = 126.0f * dpi;

        // 1. Custom 2D Saturation-Value (SV) Canvas
        const ImVec2 svPos = ImGui::GetCursorScreenPos();
        const ImRect svBb(svPos, ImVec2(svPos.x + canvasW, svPos.y + svH));

        ImGui::ItemSize(svBb);
        const ImGuiID svId = ImGui::GetID("##SVBox");
        ImGui::ItemAdd(svBb, svId);

        bool svHovered = false, svHeld = false;
        ImGui::ButtonBehavior(svBb, svId, &svHovered, &svHeld);

        if (svHeld) {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            S = std::clamp((mouse.x - svBb.Min.x) / canvasW, 0.0f, 1.0f);
            V = 1.0f - std::clamp((mouse.y - svBb.Min.y) / svH, 0.0f, 1.0f);
            ImGui::ColorConvertHSVtoRGB(H, S, V, color->x, color->y, color->z);
            changed = true;
        }

        float pureR = 0.0f, pureG = 0.0f, pureB = 0.0f;
        ImGui::ColorConvertHSVtoRGB(H, 1.0f, 1.0f, pureR, pureG, pureB);
        const ImU32 colPure = ImColor(pureR, pureG, pureB, 1.0f);
        const ImU32 colWhite = IM_COL32(255, 255, 255, 255);
        const ImU32 colBlack = IM_COL32(0, 0, 0, 255);

        popDraw->AddRectFilledMultiColor(svBb.Min, svBb.Max, colWhite, colPure, colPure, colWhite);
        popDraw->AddRectFilledMultiColor(svBb.Min, svBb.Max, 0, 0, colBlack, colBlack);
        popDraw->AddRect(svBb.Min, svBb.Max, ImColor(colors.BorderStrong), 4.0f * dpi, 0, 1.0f);

        const ImVec2 crossPos(svBb.Min.x + S * canvasW, svBb.Min.y + (1.0f - V) * svH);
        popDraw->AddCircleFilled(crossPos, 5.0f * dpi, ImColor(color->x, color->y, color->z, 1.0f), 16);
        popDraw->AddCircle(crossPos, 5.5f * dpi, IM_COL32(0, 0, 0, 190), 16, 2.0f * dpi);
        popDraw->AddCircle(crossPos, 5.0f * dpi, IM_COL32(255, 255, 255, 255), 16, 1.5f * dpi);

        ImGui::Spacing();

        // 2. Custom Hue Slider Bar
        const float hueBarH = 12.0f * dpi;
        const ImVec2 huePos = ImGui::GetCursorScreenPos();
        const ImRect hueBb(huePos, ImVec2(huePos.x + canvasW, huePos.y + hueBarH));

        ImGui::ItemSize(hueBb);
        const ImGuiID hueId = ImGui::GetID("##HueBar");
        ImGui::ItemAdd(hueBb, hueId);

        bool hueHovered = false, hueHeld = false;
        ImGui::ButtonBehavior(hueBb, hueId, &hueHovered, &hueHeld);

        if (hueHeld) {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            H = std::clamp((mouse.x - hueBb.Min.x) / canvasW, 0.0f, 1.0f);
            s_hueCache[id] = H;
            ImGui::ColorConvertHSVtoRGB(H, S, V, color->x, color->y, color->z);
            changed = true;
        }

        const ImColor hueColors[7] = {
            ImColor(255, 0, 0),
            ImColor(255, 255, 0),
            ImColor(0, 255, 0),
            ImColor(0, 255, 255),
            ImColor(0, 0, 255),
            ImColor(255, 0, 255),
            ImColor(255, 0, 0)
        };
        const float segW = canvasW / 6.0f;
        for (int i = 0; i < 6; ++i) {
            popDraw->AddRectFilledMultiColor(
                ImVec2(hueBb.Min.x + i * segW, hueBb.Min.y),
                ImVec2(hueBb.Min.x + (i + 1) * segW, hueBb.Max.y),
                hueColors[i], hueColors[i + 1], hueColors[i + 1], hueColors[i]
            );
        }
        popDraw->AddRect(hueBb.Min, hueBb.Max, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);

        const float curHueX = hueBb.Min.x + H * canvasW;
        popDraw->AddRectFilled(
            ImVec2(curHueX - 3.0f * dpi, hueBb.Min.y - 1.5f * dpi),
            ImVec2(curHueX + 3.0f * dpi, hueBb.Max.y + 1.5f * dpi),
            IM_COL32(255, 255, 255, 255),
            2.5f * dpi
        );
        popDraw->AddRect(
            ImVec2(curHueX - 3.0f * dpi, hueBb.Min.y - 1.5f * dpi),
            ImVec2(curHueX + 3.0f * dpi, hueBb.Max.y + 1.5f * dpi),
            IM_COL32(0, 0, 0, 190),
            2.5f * dpi,
            0,
            1.2f * dpi
        );

        // 3. Custom Alpha Slider Bar (if enabled)
        if (hasAlpha) {
            ImGui::Spacing();
            const float alphaBarH = 12.0f * dpi;
            const ImVec2 alphaPos = ImGui::GetCursorScreenPos();
            const ImRect alphaBb(alphaPos, ImVec2(alphaPos.x + canvasW, alphaPos.y + alphaBarH));

            ImGui::ItemSize(alphaBb);
            const ImGuiID alphaId = ImGui::GetID("##AlphaBar");
            ImGui::ItemAdd(alphaBb, alphaId);

            bool alphaHovered = false, alphaHeld = false;
            ImGui::ButtonBehavior(alphaBb, alphaId, &alphaHovered, &alphaHeld);

            if (alphaHeld) {
                const ImVec2 mouse = ImGui::GetIO().MousePos;
                color->w = std::clamp((mouse.x - alphaBb.Min.x) / canvasW, 0.0f, 1.0f);
                changed = true;
            }

            DrawCheckerboard(popDraw, alphaBb, 3.5f * dpi);

            const ImColor colA0(color->x, color->y, color->z, 0.0f);
            const ImColor colA1(color->x, color->y, color->z, 1.0f);
            popDraw->AddRectFilledMultiColor(alphaBb.Min, alphaBb.Max, colA0, colA1, colA1, colA0);
            popDraw->AddRect(alphaBb.Min, alphaBb.Max, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);

            const float curAlphaX = alphaBb.Min.x + color->w * canvasW;
            popDraw->AddRectFilled(
                ImVec2(curAlphaX - 3.0f * dpi, alphaBb.Min.y - 1.5f * dpi),
                ImVec2(curAlphaX + 3.0f * dpi, alphaBb.Max.y + 1.5f * dpi),
                IM_COL32(255, 255, 255, 255),
                2.5f * dpi
            );
            popDraw->AddRect(
                ImVec2(curAlphaX - 3.0f * dpi, alphaBb.Min.y - 1.5f * dpi),
                ImVec2(curAlphaX + 3.0f * dpi, alphaBb.Max.y + 1.5f * dpi),
                IM_COL32(0, 0, 0, 190),
                2.5f * dpi,
                0,
                1.2f * dpi
            );
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 4. HEX & RGB Integer Inputs
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.WidgetBg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, colors.WidgetHover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, colors.WidgetActive);

        char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "%s", ColorToHex(*color, hasAlpha).c_str());

        ImGui::SetNextItemWidth(92.0f * dpi);
        if (ImGui::InputText("##HexInput", hexBuf, sizeof(hexBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (HexToColor(hexBuf, *color, hasAlpha)) {
                changed = true;
            }
        }

        ImGui::SameLine(0.0f, 6.0f * dpi);

        int r = static_cast<int>(color->x * 255.0f + 0.5f);
        int g = static_cast<int>(color->y * 255.0f + 0.5f);
        int b = static_cast<int>(color->z * 255.0f + 0.5f);

        const float rgbW = (canvasW - 92.0f * dpi - 14.0f * dpi) / 3.0f;
        ImGui::SetNextItemWidth(rgbW);
        if (ImGui::DragInt("##R", &r, 1.0f, 0, 255, "R:%d", ImGuiSliderFlags_AlwaysClamp)) {
            color->x = std::clamp(r, 0, 255) / 255.0f;
            changed = true;
        }
        ImGui::SameLine(0.0f, 4.0f * dpi);
        ImGui::SetNextItemWidth(rgbW);
        if (ImGui::DragInt("##G", &g, 1.0f, 0, 255, "G:%d", ImGuiSliderFlags_AlwaysClamp)) {
            color->y = std::clamp(g, 0, 255) / 255.0f;
            changed = true;
        }
        ImGui::SameLine(0.0f, 4.0f * dpi);
        ImGui::SetNextItemWidth(rgbW);
        if (ImGui::DragInt("##B", &b, 1.0f, 0, 255, "B:%d", ImGuiSliderFlags_AlwaysClamp)) {
            color->z = std::clamp(b, 0, 255) / 255.0f;
            changed = true;
        }

        ImGui::PopStyleColor(3);
        ImGui::Spacing();

        // 5. Restraint-First Saved Swatches (Right-click any swatch to overwrite with current color)
        static ImVec4 s_savedSwatches[8] = {
            ImVec4(0.471f, 0.631f, 0.545f, 1.0f), // Muted Sage (#78A18B)
            ImVec4(0.710f, 0.514f, 0.416f, 1.0f), // Desaturated Copper (#B5836A)
            ImVec4(0.431f, 0.565f, 0.678f, 1.0f), // Subtle Steel Blue (#6E90AD)
            ImVec4(0.557f, 0.592f, 0.651f, 1.0f), // Monochrome Slate (#8E97A6)
            ImVec4(0.310f, 0.659f, 0.471f, 1.0f), // Botanical Emerald (#4FA878)
            ImVec4(0.831f, 0.608f, 0.294f, 1.0f), // Warm Ochre (#D49B4B)
            ImVec4(0.788f, 0.337f, 0.337f, 1.0f), // Terracotta Crimson (#C95656)
            ImVec4(0.925f, 0.937f, 0.957f, 1.0f), // Alabaster (#ECEFF4)
        };

        const float swatchGap = 5.0f * dpi;
        const float itemSwatchW = (canvasW - swatchGap * 7.0f) / 8.0f;

        for (size_t p = 0; p < 8; ++p) {
            if (p > 0) ImGui::SameLine(0.0f, swatchGap);

            char swatchBtnId[32];
            snprintf(swatchBtnId, sizeof(swatchBtnId), "##Preset_%zu", p);

            const ImVec2 swPos = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton(swatchBtnId, ImVec2(itemSwatchW, itemSwatchW))) {
                color->x = s_savedSwatches[p].x;
                color->y = s_savedSwatches[p].y;
                color->z = s_savedSwatches[p].z;
                if (!hasAlpha) color->w = 1.0f;
                changed = true;
            }
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    s_savedSwatches[p] = *color;
                }
                ImGui::SetTooltip("Click to apply  |  Right-click to save current color");
            }

            const ImVec2 swMax(swPos.x + itemSwatchW, swPos.y + itemSwatchW);
            popDraw->AddRectFilled(swPos, swMax, ImColor(s_savedSwatches[p]), 4.0f * dpi);
            popDraw->AddRect(
                swPos,
                swMax,
                ImColor(ImGui::IsItemHovered() ? colors.Accent : colors.BorderStrong),
                4.0f * dpi,
                0,
                ImGui::IsItemHovered() ? 1.5f : 1.0f
            );
        }

        ImGui::Spacing();

        // 6. Styled Copy & Paste Row
        ImGui::PushStyleColor(ImGuiCol_Button, colors.WidgetBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.WidgetHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors.WidgetActive);
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f * dpi);

        const float btnW = (canvasW - 8.0f * dpi) * 0.5f;
        if (ImGui::Button("Copy HEX", ImVec2(btnW, 24.0f * dpi))) {
            ImGui::SetClipboardText(ColorToHex(*color, hasAlpha).c_str());
        }
        ImGui::SameLine(0.0f, 8.0f * dpi);
        if (ImGui::Button("Paste HEX", ImVec2(btnW, 24.0f * dpi))) {
            const char* clip = ImGui::GetClipboardText();
            if (HexToColor(clip, *color, hasAlpha)) {
                changed = true;
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);

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

bool ColorPickerRow(const char* label, ImVec4* color, bool hasAlpha) {
    if (!color) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const float availW = ImGui::GetContentRegionAvail().x;
    const float rowH = metrics.RowHeight * dpi;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect rowBb(pos, ImVec2(pos.x + availW, pos.y + rowH));
    ImGui::ItemSize(rowBb);
    const ImVec2 nextRowCursor = ImGui::GetCursorScreenPos();

    PushFont(FontWeight::Medium);
    const char* textEnd = ImGui::FindRenderedTextEnd(label);
    const ImVec2 labelSz = ImGui::CalcTextSize(label, textEnd);
    drawList->AddText(
        nullptr,
        0.0f,
        ImVec2(pos.x + 4.0f * dpi, pos.y + (rowH - labelSz.y) * 0.5f),
        ImColor(colors.TextPrimary),
        label,
        textEnd
    );
    PopFont();

    // Hex readout badge + color swatch on the right
    const std::string hexStr = ColorToHex(*color, false);
    PushFont(FontWeight::Caption);
    const ImVec2 hexSz = ImGui::CalcTextSize(hexStr.c_str());
    const float swatchW = 26.0f * dpi;
    const float swatchH = 20.0f * dpi;
    const float hexX = pos.x + availW - swatchW - hexSz.x - 14.0f * dpi;
    drawList->AddText(ImVec2(hexX, pos.y + (rowH - hexSz.y) * 0.5f), ImColor(colors.TextSecondary), hexStr.c_str());
    PopFont();

    ImGui::SetCursorScreenPos(ImVec2(pos.x + availW - swatchW - 4.0f * dpi, pos.y + (rowH - swatchH) * 0.5f));
    char pickerId[96];
    snprintf(pickerId, sizeof(pickerId), "##CPRow_%s", label);
    const bool changed = ColorPicker(pickerId, color, hasAlpha, ImVec2(swatchW, swatchH));

    ImGui::SetCursorScreenPos(nextRowCursor);
    window->DC.IsSetPos = false;
    return changed;
}

} // namespace parallax
