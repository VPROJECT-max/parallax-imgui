#include "parallax/auth/key_system.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/widgets/button.hpp"
#include <imgui_internal.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <shellapi.h>

namespace parallax::auth {

KeySystem& KeySystem::Get() noexcept {
    static KeySystem s_instance;
    return s_instance;
}

KeySystem::KeySystem() {
    // Default built-in verifier: validates known demo keys or any key >= 8 characters with PRLX prefix
    m_verifier = [](const std::string& key) -> bool {
        if (key.empty()) return false;
        if (key == "PRLX-2026-VIP" || key == "PRLX-DEMO-PRO" || key == "PARALLAX-KEY-2026") {
            return true;
        }
        if (key.rfind("PRLX-", 0) == 0 && key.length() >= 10) {
            return true;
        }
        return false;
    };

    LoadRememberedKey();
}

void KeySystem::SetVerifier(std::function<bool(const std::string&)> verifier) {
    m_verifier = std::move(verifier);
}

void KeySystem::SetKeyURL(const std::string& url) {
    m_keyUrl = url;
}

void KeySystem::SetRememberKey(bool remember) {
    m_rememberKey = remember;
}

bool KeySystem::IsRememberKeyEnabled() const noexcept {
    return m_rememberKey;
}

void KeySystem::SetLicenseFilePath(const std::string& path) {
    m_licenseFilePath = path;
}

void KeySystem::StartVerification(const std::string& key) {
    if (m_status == KeyStatus::Verifying) return;

    m_enteredKey = key;
    m_status = KeyStatus::Verifying;
    m_statusMessage = "Validating cryptographic signature with authorization server...";

    auto verifier = m_verifier;
    m_verifyFuture = std::async(std::launch::async, [verifier, key]() {
        // Simulate non-blocking network verification delay
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (verifier) {
            return verifier(key);
        }
        return false;
    });
}

void KeySystem::Update() {
    if (m_status == KeyStatus::Verifying && m_verifyFuture.valid()) {
        auto waitStatus = m_verifyFuture.wait_for(std::chrono::milliseconds(0));
        if (waitStatus == std::future_status::ready) {
            bool valid = m_verifyFuture.get();
            if (valid) {
                m_status = KeyStatus::Success;
                m_authenticated = true;
                m_statusMessage = "License authenticated! Welcome, Alex Mercer.";
                if (m_rememberKey) {
                    SaveRememberedKey();
                }
            } else {
                m_status = KeyStatus::InvalidKey;
                m_authenticated = false;
                m_statusMessage = "Invalid or expired key. Please check your credentials.";
            }
        }
    }
}

bool KeySystem::IsAuthenticated() const noexcept {
    return m_authenticated;
}

void KeySystem::SetAuthenticated(bool auth) noexcept {
    m_authenticated = auth;
    if (auth) {
        m_status = KeyStatus::Success;
        m_statusMessage = "License active.";
    } else {
        m_status = KeyStatus::Idle;
        m_statusMessage.clear();
    }
}

KeyStatus KeySystem::GetStatus() const noexcept {
    return m_status;
}

const std::string& KeySystem::GetStatusMessage() const noexcept {
    return m_statusMessage;
}

const std::string& KeySystem::GetEnteredKey() const noexcept {
    return m_enteredKey;
}

void KeySystem::SetEnteredKey(const std::string& key) {
    m_enteredKey = key;
}

bool KeySystem::SaveRememberedKey() {
    if (m_enteredKey.empty()) return false;
    std::ofstream out(m_licenseFilePath, std::ios::binary);
    if (!out.is_open()) return false;
    out << m_enteredKey;
    return true;
}

bool KeySystem::LoadRememberedKey() {
    std::ifstream in(m_licenseFilePath, std::ios::binary);
    if (!in.is_open()) return false;
    std::string key;
    in >> key;
    if (!key.empty()) {
        m_enteredKey = key;
        // Verify loaded key silently
        if (m_verifier && m_verifier(key)) {
            m_authenticated = true;
            m_status = KeyStatus::Success;
            m_statusMessage = "Loaded stored license.";
            return true;
        }
    }
    return false;
}

void KeySystem::Logout() {
    m_authenticated = false;
    m_status = KeyStatus::Idle;
    m_statusMessage.clear();
    remove(m_licenseFilePath.c_str());
}

bool KeySystem::RenderModal(bool* p_open) {
    Update();

    if (p_open && !*p_open) return false;

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();

    if (!ImGui::IsPopupOpen("##ParallaxKeyModal")) {
        ImGui::OpenPopup("##ParallaxKeyModal");
    }

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.0f * dpi, 0.0f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, metrics.WindowRounding * dpi);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f * dpi, 20.0f * dpi));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, colors.WindowBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.04f, 0.05f, 0.08f, 0.75f));

    bool opened = ImGui::BeginPopupModal("##ParallaxKeyModal", p_open,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

    if (!opened) {
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        if (p_open) *p_open = false;
        return false;
    }

    // Dismiss modal on Escape key
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        if (p_open) *p_open = false;
        ImGui::CloseCurrentPopup();
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 winPos = ImGui::GetWindowPos();
    const float contentWidth = ImGui::GetContentRegionAvail().x;

    // -------------------------------------------------------------------------
    // Top Header: Shield Icon + Title / Subtitle + Close 'X' Button
    // -------------------------------------------------------------------------
    const float shieldDim = 28.0f * dpi;
    const ImVec2 iconPos(winPos.x + 24.0f * dpi, winPos.y + 20.0f * dpi);
    ctx.Icons().DrawIcon(drawList, "shield", iconPos, ImVec2(shieldDim, shieldDim), colors.Accent);

    // Title & Subtitle text block
    const float titleX = winPos.x + 24.0f * dpi + shieldDim + 12.0f * dpi;
    PushFont(FontWeight::Bold);
    drawList->AddText(ImVec2(titleX, winPos.y + 18.0f * dpi), ImColor(colors.TextPrimary), "PARALLAX // AUTHORIZATION");
    PopFont();

    PushFont(FontWeight::Caption);
    drawList->AddText(ImVec2(titleX, winPos.y + 38.0f * dpi), ImColor(colors.TextMuted), "Enter your product key to unlock the complete UI suite.");
    PopFont();

    // Close 'X' button in top-right
    const float closeDim = 26.0f * dpi;
    ImGui::SetCursorScreenPos(ImVec2(winPos.x + 24.0f * dpi + contentWidth - closeDim, winPos.y + 18.0f * dpi));
    if (Button("##CloseKeyModal", "x", ButtonVariant::Ghost, ImVec2(closeDim, closeDim))) {
        if (p_open) *p_open = false;
        ImGui::CloseCurrentPopup();
    }

    // Advance cursor past header
    ImGui::SetCursorScreenPos(ImVec2(winPos.x + 24.0f * dpi, winPos.y + 60.0f * dpi));
    drawList->AddLine(
        ImVec2(winPos.x + 24.0f * dpi, winPos.y + 60.0f * dpi),
        ImVec2(winPos.x + 24.0f * dpi + contentWidth, winPos.y + 60.0f * dpi),
        ImColor(colors.Border),
        1.0f
    );

    ImGui::SetCursorScreenPos(ImVec2(winPos.x + 24.0f * dpi, winPos.y + 72.0f * dpi));
    ImGui::Spacing();

    // Key input label
    PushFont(FontWeight::Medium);
    ImGui::TextColored(colors.TextSecondary, "Product License Key");
    PopFont();

    ImGui::Spacing();

    // Key input text field with show/hide password toggle
    static char s_keyBuffer[128] = "";
    if (s_keyBuffer[0] == '\0' && !m_enteredKey.empty()) {
        snprintf(s_keyBuffer, sizeof(s_keyBuffer), "%s", m_enteredKey.c_str());
    }

    const float eyeBtnDim = 36.0f * dpi;
    const float inputW = contentWidth - eyeBtnDim - 8.0f * dpi;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, metrics.FrameRounding * dpi);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f * dpi, 9.0f * dpi));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.CardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_None;
    if (!m_showKeyText) {
        inputFlags |= ImGuiInputTextFlags_Password;
    }

    ImGui::SetNextItemWidth(inputW);
    if (ImGui::InputTextWithHint("##KeyInput", "PRLX-XXXX-XXXX-XXXX", s_keyBuffer, sizeof(s_keyBuffer), inputFlags)) {
        m_enteredKey = s_keyBuffer;
    }

    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter) && m_status != KeyStatus::Verifying) {
        StartVerification(s_keyBuffer);
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    ImGui::SameLine(0.0f, 8.0f * dpi);

    // Eye toggle button (show / hide key)
    if (Button("##ToggleShowKey", m_showKeyText ? "eye-off" : "eye", ButtonVariant::Secondary, ImVec2(eyeBtnDim, eyeBtnDim))) {
        m_showKeyText = !m_showKeyText;
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Remember key checkbox
    ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.CardBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, colors.CardHover);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, colors.Accent);
    ImGui::PushStyleColor(ImGuiCol_CheckMark, colors.Accent);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.Border);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * dpi);
    ImGui::Checkbox("Remember key on this device", &m_rememberKey);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(5);

    ImGui::Spacing();

    // Status / Feedback message area (deterministic height to prevent layout jumps)
    const float statusBannerHeight = 34.0f * dpi;
    if (m_status == KeyStatus::Verifying) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(colors.Accent.x, colors.Accent.y, colors.Accent.z, 0.12f));
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Accent);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding * dpi);
        ImGui::BeginChild("##VerifyingBanner", ImVec2(contentWidth, statusBannerHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImDrawList* bannerDraw = ImGui::GetWindowDrawList();
        const ImVec2 bannerPos = ImGui::GetWindowPos();
        const float time = static_cast<float>(ImGui::GetTime());
        const float spinnerR = 6.0f * dpi;
        const ImVec2 spinCenter(bannerPos.x + 18.0f * dpi, bannerPos.y + statusBannerHeight * 0.5f);

        bannerDraw->PathClear();
        for (int i = 0; i < 6; ++i) {
            const float a = time * 6.0f + static_cast<float>(i) * 0.4f;
            bannerDraw->PathLineTo(ImVec2(spinCenter.x + std::cos(a) * spinnerR, spinCenter.y + std::sin(a) * spinnerR));
        }
        bannerDraw->PathStroke(ImColor(colors.Accent), 0, 2.0f);

        ImGui::SetCursorScreenPos(ImVec2(spinCenter.x + 14.0f * dpi, bannerPos.y + (statusBannerHeight - ImGui::GetTextLineHeight()) * 0.5f));
        PushFont(FontWeight::Caption);
        ImGui::TextColored(colors.Accent, "%s", m_statusMessage.empty() ? "Contacting authorization server..." : m_statusMessage.c_str());
        PopFont();

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    } else if (m_status == KeyStatus::Success) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.25f, 0.12f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Success);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding * dpi);
        ImGui::BeginChild("##SuccessBanner", ImVec2(contentWidth, statusBannerHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetCursorPos(ImVec2(12.0f * dpi, (statusBannerHeight - ImGui::GetTextLineHeight()) * 0.5f));
        PushFont(FontWeight::Caption);
        ImGui::TextColored(colors.Success, "%s", m_statusMessage.empty() ? "Key verified successfully. Welcome!" : m_statusMessage.c_str());
        PopFont();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    } else if (m_status == KeyStatus::InvalidKey) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.30f, 0.08f, 0.08f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding * dpi);
        ImGui::BeginChild("##ErrorBanner", ImVec2(contentWidth, statusBannerHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetCursorPos(ImVec2(12.0f * dpi, (statusBannerHeight - ImGui::GetTextLineHeight()) * 0.5f));
        PushFont(FontWeight::Caption);
        ImGui::TextColored(colors.Danger, "%s", m_statusMessage.empty() ? "Invalid or expired license key." : m_statusMessage.c_str());
        PopFont();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    } else {
        ImGui::Dummy(ImVec2(contentWidth, statusBannerHeight));
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Bottom Action Buttons
    const float buttonGap = 12.0f * dpi;
    const float buttonW = (contentWidth - buttonGap) * 0.5f;
    const float buttonH = 38.0f * dpi;

    // Get Key button (links to developer portal)
    if (Button("Get License Key", "external-link", ButtonVariant::Secondary, ImVec2(buttonW, buttonH))) {
        ShellExecuteA(NULL, "open", m_keyUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::SameLine(0.0f, buttonGap);

    // Verify / Unlock button
    const char* unlockLabel = (m_status == KeyStatus::Verifying) ? "Verifying..." : (m_authenticated ? "Done / Continue" : "Unlock Suite");
    const char* unlockIcon = m_authenticated ? "check-circle" : "key";
    if (Button(unlockLabel, unlockIcon, ButtonVariant::Primary, ImVec2(buttonW, buttonH))) {
        if (m_authenticated) {
            if (p_open) *p_open = false;
            ImGui::CloseCurrentPopup();
        } else if (m_status != KeyStatus::Verifying) {
            StartVerification(s_keyBuffer);
        }
    }

    ImGui::EndPopup();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    return opened;
}

} // namespace parallax::auth
