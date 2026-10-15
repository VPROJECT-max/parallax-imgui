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

// Retain hue when saturation or value drops to 0
static std::unordered_map<ImGuiID, float> s_hueCache;

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

    // Color Picker Popup positioning (upward/downward check)
    const float popupW = 236.0f * dpi;
    const float popupH = hasAlpha ? (320.0f * dpi) : (295.0f * dpi);

    const float winBottom = window->ClipRect.Max.y;
    const float screenH = ImGui::GetIO().DisplaySize.y;
    const float bottomLimit = (winBottom > 0.0f && winBottom < screenH) ? winBottom : screenH;

    ImVec2 popupPos;
    if (bb.Max.y + popupH > bottomLimit - 12.0f * dpi && bb.Min.y - popupH > 10.0f * dpi) {
        popupPos = ImVec2(bb.Min.x - (popupW - swatchSize.x), bb.Min.y - popupH - 4.0f * dpi);
    } else {
        popupPos = ImVec2(bb.Min.x - (popupW - swatchSize.x), bb.Max.y + 4.0f * dpi);
    }

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(popupW, 0.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f * dpi, 12.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.HeaderBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId)) {
        PushFont(FontWeight::Medium);
        ImGui::TextColored(colors.TextPrimary, "Color Palette");
        PopFont();
        ImGui::Spacing();

        // Convert current color to HSV
        float H = 0.0f, S = 0.0f, V = 0.0f;
        ImGui::ColorConvertRGBtoHSV(color->x, color->y, color->z, H, S, V);

        // Retrieve or store cached hue to prevent snapping at zero saturation
        if (s_hueCache.find(id) == s_hueCache.end()) {
            s_hueCache[id] = H;
        } else if (S > 0.02f && V > 0.02f) {
            s_hueCache[id] = H;
        } else {
            H = s_hueCache[id];
        }

        const float canvasW = popupW - 24.0f * dpi;
        const float svH = 125.0f * dpi;

        ImDrawList* popDraw = ImGui::GetWindowDrawList();

        // ==========================================
        // 1. Custom 2D Saturation-Value (SV) Box
        // ==========================================
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

        // Draw SV Box with multi-color gradients
        float pureR = 0.0f, pureG = 0.0f, pureB = 0.0f;
        ImGui::ColorConvertHSVtoRGB(H, 1.0f, 1.0f, pureR, pureG, pureB);
        const ImU32 colPure = ImColor(pureR, pureG, pureB, 1.0f);
        const ImU32 colWhite = IM_COL32(255, 255, 255, 255);
        const ImU32 colBlack = IM_COL32(0, 0, 0, 255);

        popDraw->AddRectFilledMultiColor(svBb.Min, svBb.Max, colWhite, colPure, colPure, colWhite);
        // Vertical black gradient overlay
        popDraw->AddRectFilledMultiColor(svBb.Min, svBb.Max, 0, 0, colBlack, colBlack);
        popDraw->AddRect(svBb.Min, svBb.Max, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);

        // Crosshair indicator
        const ImVec2 crossPos = ImVec2(svBb.Min.x + S * canvasW, svBb.Min.y + (1.0f - V) * svH);
        popDraw->AddCircle(crossPos, 4.5f * dpi, IM_COL32(0, 0, 0, 200), 12, 2.0f * dpi);
        popDraw->AddCircle(crossPos, 4.0f * dpi, IM_COL32(255, 255, 255, 255), 12, 1.5f * dpi);

        ImGui::Spacing();

        // ==========================================
        // 2. Custom Hue Slider Bar
        // ==========================================
        const float hueBarH = 13.0f * dpi;
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

        // Draw 6 spectrum gradient segments
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

        // Hue cursor pill
        const float curHueX = hueBb.Min.x + H * canvasW;
        popDraw->AddRectFilled(ImVec2(curHueX - 2.5f * dpi, hueBb.Min.y - 1.0f * dpi), ImVec2(curHueX + 2.5f * dpi, hueBb.Max.y + 1.0f * dpi), IM_COL32(255, 255, 255, 255), 2.0f * dpi);
        popDraw->AddRect(ImVec2(curHueX - 2.5f * dpi, hueBb.Min.y - 1.0f * dpi), ImVec2(curHueX + 2.5f * dpi, hueBb.Max.y + 1.0f * dpi), IM_COL32(0, 0, 0, 180), 2.0f * dpi, 0, 1.0f);

        // ==========================================
        // 3. Custom Alpha Slider Bar (if enabled)
        // ==========================================
        if (hasAlpha) {
            ImGui::Spacing();
            const float alphaBarH = 13.0f * dpi;
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

            // Checkerboard underlay
            DrawCheckerboard(popDraw, alphaBb, 3.5f * dpi);

            // Alpha gradient from 0 to 1
            const ImColor colA0 = ImColor(color->x, color->y, color->z, 0.0f);
            const ImColor colA1 = ImColor(color->x, color->y, color->z, 1.0f);
            popDraw->AddRectFilledMultiColor(alphaBb.Min, alphaBb.Max, colA0, colA1, colA1, colA0);
            popDraw->AddRect(alphaBb.Min, alphaBb.Max, ImColor(colors.Border), 3.0f * dpi, 0, 1.0f);

            // Alpha cursor pill
            const float curAlphaX = alphaBb.Min.x + color->w * canvasW;
            popDraw->AddRectFilled(ImVec2(curAlphaX - 2.5f * dpi, alphaBb.Min.y - 1.0f * dpi), ImVec2(curAlphaX + 2.5f * dpi, alphaBb.Max.y + 1.0f * dpi), IM_COL32(255, 255, 255, 255), 2.0f * dpi);
            popDraw->AddRect(ImVec2(curAlphaX - 2.5f * dpi, alphaBb.Min.y - 1.0f * dpi), ImVec2(curAlphaX + 2.5f * dpi, alphaBb.Max.y + 1.0f * dpi), IM_COL32(0, 0, 0, 180), 2.0f * dpi, 0, 1.0f);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ==========================================
        // 4. Hex & RGB Integer Inputs
        // ==========================================
        char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "%s", ColorToHex(*color, hasAlpha).c_str());

        ImGui::SetNextItemWidth(90.0f * dpi);
        if (ImGui::InputText("##HexInput", hexBuf, sizeof(hexBuf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (HexToColor(hexBuf, *color, hasAlpha)) {
                changed = true;
            }
        }

        ImGui::SameLine(0.0f, 8.0f * dpi);

        int r = static_cast<int>(color->x * 255.0f + 0.5f);
        int g = static_cast<int>(color->y * 255.0f + 0.5f);
        int b = static_cast<int>(color->z * 255.0f + 0.5f);

        const float rgbW = 32.0f * dpi;
        ImGui::SetNextItemWidth(rgbW);
        if (ImGui::DragInt("##R", &r, 1.0f, 0, 255, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            color->x = std::clamp(r, 0, 255) / 255.0f;
            changed = true;
        }
        ImGui::SameLine(0.0f, 4.0f * dpi);
        ImGui::SetNextItemWidth(rgbW);
        if (ImGui::DragInt("##G", &g, 1.0f, 0, 255, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            color->y = std::clamp(g, 0, 255) / 255.0f;
            changed = true;
        }
        ImGui::SameLine(0.0f, 4.0f * dpi);
        ImGui::SetNextItemWidth(rgbW);
        if (ImGui::DragInt("##B", &b, 1.0f, 0, 255, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            color->z = std::clamp(b, 0, 255) / 255.0f;
            changed = true;
        }

        ImGui::Spacing();

        // ==========================================
        // 5. Preset Palette Swatches
        // ==========================================
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
            if (p > 0) ImGui::SameLine(0.0f, 5.0f * dpi);

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

        // ==========================================
        // 6. Styled Copy & Paste Row
        // ==========================================
        ImGui::PushStyleColor(ImGuiCol_Button, colors.CardBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.CardHover);
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, metrics.FrameRounding);

        const float btnW = (canvasW - 8.0f * dpi) * 0.5f;
        if (ImGui::Button("Copy HEX", ImVec2(btnW, 22.0f * dpi))) {
            ImGui::SetClipboardText(ColorToHex(*color, hasAlpha).c_str());
        }
        ImGui::SameLine(0.0f, 8.0f * dpi);
        if (ImGui::Button("Paste HEX", ImVec2(btnW, 22.0f * dpi))) {
            const char* clip = ImGui::GetClipboardText();
            if (HexToColor(clip, *color, hasAlpha)) {
                changed = true;
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);

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
