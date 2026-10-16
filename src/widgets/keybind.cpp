#include "parallax/widgets/keybind.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/input/hotkeys.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include <imgui_internal.h>
#include <windows.h>
#include <unordered_map>

namespace parallax {

std::string GetKeyName(int vkey) {
    if (vkey == 0) return "NONE";

    switch (vkey) {
        case VK_LBUTTON:  return "M1";
        case VK_RBUTTON:  return "M2";
        case VK_MBUTTON:  return "M3";
        case VK_XBUTTON1: return "M4";
        case VK_XBUTTON2: return "M5";
        case VK_SPACE:    return "SPACE";
        case VK_TAB:      return "TAB";
        case VK_RETURN:   return "ENTER";
        case VK_ESCAPE:   return "ESC";
        case VK_BACK:     return "BACK";
        case VK_SHIFT:    return "SHIFT";
        case VK_LSHIFT:   return "LSHIFT";
        case VK_RSHIFT:   return "RSHIFT";
        case VK_CONTROL:  return "CTRL";
        case VK_LCONTROL: return "LCTRL";
        case VK_RCONTROL: return "RCTRL";
        case VK_MENU:     return "ALT";
        case VK_LMENU:    return "LALT";
        case VK_RMENU:    return "RALT";
        case VK_CAPITAL:  return "CAPS";
        case VK_INSERT:   return "INS";
        case VK_DELETE:   return "DEL";
        case VK_HOME:     return "HOME";
        case VK_END:      return "END";
        case VK_PRIOR:    return "PGUP";
        case VK_NEXT:     return "PGDN";
        case VK_UP:       return "UP";
        case VK_DOWN:     return "DOWN";
        case VK_LEFT:     return "LEFT";
        case VK_RIGHT:    return "RIGHT";
        default: break;
    }

    if (vkey >= VK_F1 && vkey <= VK_F12) {
        return "F" + std::to_string(vkey - VK_F1 + 1);
    }

    if ((vkey >= '0' && vkey <= '9') || (vkey >= 'A' && vkey <= 'Z')) {
        return std::string(1, static_cast<char>(vkey));
    }

    // Try Windows MapVirtualKeyA
    UINT scanCode = MapVirtualKeyA(static_cast<UINT>(vkey), MAPVK_VK_TO_VSC);
    char keyNameBuf[64]{};
    int len = GetKeyNameTextA(scanCode << 16, keyNameBuf, sizeof(keyNameBuf));
    if (len > 0) {
        return std::string(keyNameBuf);
    }

    return "KEY_" + std::to_string(vkey);
}

static std::unordered_map<int, bool> s_simulatedKeys;
static std::unordered_map<const Keybind*, bool> s_bindPrevDown;

void SimulateKeyState(int vkey, bool isDown) {
    s_simulatedKeys[vkey] = isDown;
}

void ClearSimulatedKeys() {
    s_simulatedKeys.clear();
    s_bindPrevDown.clear();
}

static bool IsKeyActiveInternal(int vkey) {
    auto it = s_simulatedKeys.find(vkey);
    if (it != s_simulatedKeys.end()) {
        return it->second;
    }
    return (GetAsyncKeyState(vkey) & 0x8000) != 0;
}

bool UpdateKeybindState(Keybind& bind) {
    if (bind.mode == KeybindMode::AlwaysOn) {
        bind.active = true;
        return true;
    }

    if (bind.vkey == 0) {
        bind.active = false;
        return false;
    }

    // When global key capture is active, suppress state toggling
    if (Context::Get().Input().IsKeyCaptureActive()) {
        return bind.active;
    }

    const bool isDown = IsKeyActiveInternal(bind.vkey);
    const bool wasDown = s_bindPrevDown[&bind];
    s_bindPrevDown[&bind] = isDown;

    if (bind.mode == KeybindMode::Hold) {
        bind.active = isDown;
    } else if (bind.mode == KeybindMode::Toggle) {
        if (isDown && !wasDown) {
            bind.active = !bind.active;
        }
    }

    return bind.active;
}

static ImGuiID s_listeningWidgetId = 0;
static bool s_justStartedListening = false;
static BYTE s_prevKeyboardState[256] = {};

bool KeybindWidget(const char* str_id, Keybind* bind, const ImVec2& size) {
    if (!bind) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id = window->GetID(str_id);
    const bool isListening = (s_listeningWidgetId == id);

    // Format display string e.g. "F" or "NONE" or "..."
    std::string btnText;
    if (isListening) {
        btnText = "...";
    } else if (bind->vkey == 0) {
        btnText = "NONE";
    } else {
        btnText = GetKeyName(bind->vkey);
    }

    ImVec2 textSize = ImGui::CalcTextSize(btnText.c_str());
    ImVec2 widgetSize = size;
    if (widgetSize.x <= 0.0f) widgetSize.x = std::max(textSize.x + 18.0f * dpi, 48.0f * dpi);
    if (widgetSize.y <= 0.0f) widgetSize.y = 22.0f * dpi;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImRect bb(pos, ImVec2(pos.x + widgetSize.x, pos.y + widgetSize.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Animated hover/listening state
    float targetHighlight = isListening ? 1.0f : (hovered ? 0.45f : 0.0f);
    float highlight = ctx.Anim().SpringFloat(id, targetHighlight, {180.0f, 22.0f, 1.0f});
    (void)highlight;

    const float rounding = 4.5f * dpi;

    // Tactile keycap pill surface
    ImVec4 bgCol = isListening
        ? colors.AccentMuted
        : (hovered ? colors.WidgetHover : colors.WidgetBg);
    drawList->AddRectFilled(bb.Min, bb.Max, ImColor(bgCol), rounding);

    // Keycap bottom bevel line for hardware depth
    if (!isListening) {
        drawList->AddLine(
            ImVec2(bb.Min.x + 3.0f * dpi, bb.Max.y - 1.5f * dpi),
            ImVec2(bb.Max.x - 3.0f * dpi, bb.Max.y - 1.5f * dpi),
            ImColor(colors.BorderStrong),
            1.0f * dpi
        );
    }

    // Border
    ImVec4 borderCol = isListening
        ? colors.Accent
        : (bind->active ? colors.Accent : (hovered ? colors.BorderStrong : colors.Border));
    drawList->AddRect(bb.Min, bb.Max, ImColor(borderCol), rounding, 0, (isListening || bind->active) ? 1.5f : 1.0f);

    // Text label
    ImVec4 textCol = isListening
        ? colors.Accent
        : (bind->active ? colors.Accent : (bind->vkey == 0 ? colors.TextMuted : colors.TextPrimary));
    ImVec2 textPos = ImVec2(
        bb.Min.x + (widgetSize.x - textSize.x) * 0.5f,
        bb.Min.y + (widgetSize.y - textSize.y) * 0.5f - 0.5f * dpi
    );
    drawList->AddText(textPos, ImColor(textCol), btnText.c_str());

    // Popup management
    char popupId[64];
    snprintf(popupId, sizeof(popupId), "##KBPopup_%08X", id);

    // Left-click directly triggers listening mode
    if (pressed) {
        if (!isListening) {
            s_listeningWidgetId = id;
            s_justStartedListening = true;
            ctx.Input().SetKeyCaptureActive(true);
        } else {
            s_listeningWidgetId = 0;
            ctx.Input().SetKeyCaptureActive(false);
        }
    }

    // Right-click opens mode / settings popup
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(popupId);
    }

    bool changed = false;

    // Handle key capture when listening
    if (isListening) {
        ctx.Input().SetKeyCaptureActive(true);

        if (s_justStartedListening) {
            // Snapshot current keyboard & mouse buttons so initial click isn't captured
            GetKeyboardState(s_prevKeyboardState);
            s_justStartedListening = false;
        } else {
            // Check mouse buttons
            const int mouseButtons[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
            bool caught = false;
            for (int mb : mouseButtons) {
                const bool isDown = (GetAsyncKeyState(mb) & 0x8000) != 0;
                const bool wasDown = (s_prevKeyboardState[mb] & 0x80) != 0;
                if (isDown && !wasDown) {
                    bind->vkey = mb;
                    s_listeningWidgetId = 0;
                    ctx.Input().SetKeyCaptureActive(false);
                    changed = true;
                    caught = true;
                    break;
                }
            }

            // Check keyboard keys
            if (!caught) {
                for (int k = 0x08; k <= 0xFE; ++k) {
                    if (k == VK_ESCAPE) {
                        // Escape cancels listening without changing
                        s_listeningWidgetId = 0;
                        ctx.Input().SetKeyCaptureActive(false);
                        break;
                    }
                    if (k == VK_DELETE || k == VK_BACK) {
                        // Delete / Backspace clears the keybind
                        bind->vkey = 0;
                        bind->active = false;
                        s_listeningWidgetId = 0;
                        ctx.Input().SetKeyCaptureActive(false);
                        changed = true;
                        break;
                    }

                    const bool isDown = (GetAsyncKeyState(k) & 0x8000) != 0;
                    const bool wasDown = (s_prevKeyboardState[k] & 0x80) != 0;
                    if (isDown && !wasDown) {
                        bind->vkey = k;
                        s_listeningWidgetId = 0;
                        ctx.Input().SetKeyCaptureActive(false);
                        changed = true;
                        break;
                    }
                }
            }

            GetKeyboardState(s_prevKeyboardState);
        }
    }

    // Anchored configuration popup (opened via Right-Click or from popup button)
    ImGui::SetNextWindowSize(ImVec2(196.0f * dpi, 0.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f * dpi, 10.0f * dpi));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, metrics.PopupRounding * dpi);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.PopupBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    if (ImGui::BeginPopup(popupId)) {
        ImGui::TextColored(colors.TextPrimary, "Keybind Settings");
        ImGui::Spacing();

        // Key capture button
        std::string captureBtnText = isListening ? "Press any key/button..." : ("Key: " + GetKeyName(bind->vkey));
        if (ImGui::Button(captureBtnText.c_str(), ImVec2(-1.0f, 26.0f * dpi))) {
            s_listeningWidgetId = id;
            s_justStartedListening = true;
            ctx.Input().SetKeyCaptureActive(true);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Mode Selection
        ImGui::TextColored(colors.TextMuted, "Activation Mode:");
        const char* modeNames[] = { "Hold", "Toggle", "Always" };
        int currentMode = static_cast<int>(bind->mode);

        for (int m = 0; m < 3; ++m) {
            if (m > 0) ImGui::SameLine(0.0f, 4.0f * dpi);
            bool isCurrent = (currentMode == m);

            if (isCurrent) {
                ImGui::PushStyleColor(ImGuiCol_Button, colors.AccentMuted);
                ImGui::PushStyleColor(ImGuiCol_Text, colors.Accent);
            }

            if (ImGui::Button(modeNames[m], ImVec2(52.0f * dpi, 24.0f * dpi))) {
                bind->mode = static_cast<KeybindMode>(m);
                changed = true;
            }

            if (isCurrent) {
                ImGui::PopStyleColor(2);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Clear button
        if (ImGui::Button("Clear Keybind", ImVec2(-1.0f, 22.0f * dpi))) {
            bind->vkey = 0;
            bind->active = false;
            s_listeningWidgetId = 0;
            ctx.Input().SetKeyCaptureActive(false);
            changed = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return changed;
}

bool KeybindRow(const char* label, const char* description, Keybind* bind) {
    if (!bind) return false;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();

    const float availW = ImGui::GetContentRegionAvail().x;
    const bool hasDesc = (description && description[0] != '\0');
    const float rowH = hasDesc ? (38.0f * dpi) : (28.0f * dpi);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + availW, pos.y + rowH));
    const ImGuiID id = window->GetID(label);

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    const ImVec2 nextRowCursor = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = window->DrawList;

    // Label and description
    PushFont(FontWeight::Medium);
    const float labelH = ImGui::GetFontSize();
    float textY = pos.y + (hasDesc ? (3.0f * dpi) : ((rowH - labelH) * 0.5f));
    const char* labelEnd = ImGui::FindRenderedTextEnd(label);
    drawList->AddText(nullptr, 0.0f, ImVec2(pos.x, textY), ImColor(colors.TextPrimary), label, labelEnd);
    PopFont();

    if (hasDesc) {
        PushFont(FontWeight::Caption);
        const float descH = ImGui::GetFontSize();
        const float descY = pos.y + rowH - descH - 3.0f * dpi;
        const char* descEnd = ImGui::FindRenderedTextEnd(description);
        drawList->AddText(nullptr, 0.0f, ImVec2(pos.x, descY), ImColor(colors.TextMuted), description, descEnd);
        PopFont();
    }

    // Right-aligned Keybind widget
    const float kbW = 68.0f * dpi;
    const float kbH = 22.0f * dpi;
    const float kbX = pos.x + availW - kbW;
    const float kbY = pos.y + (rowH - kbH) * 0.5f;

    ImGui::SetCursorScreenPos(ImVec2(kbX, kbY));
    char kbIdStr[64];
    snprintf(kbIdStr, sizeof(kbIdStr), "##RowKB_%08X", id);
    bool changed = KeybindWidget(kbIdStr, bind, ImVec2(kbW, kbH));

    window->DC.CursorPos = nextRowCursor;
    window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, bb.Max);
    window->DC.IsSetPos = false;

    return changed;
}

} // namespace parallax
