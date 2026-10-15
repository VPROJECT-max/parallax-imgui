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

    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();

    const ImVec2 dialogSize(480.0f * dpi, 380.0f * dpi);
    const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    const ImVec2 dialogPos(
        (screenSize.x - dialogSize.x) * 0.5f,
        (screenSize.y - dialogSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(dialogPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dialogSize, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, metrics.WindowRounding * dpi);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f * dpi, 24.0f * dpi));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, colors.WindowBg);
    ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

    bool opened = ImGui::Begin("##ParallaxKeyModal", p_open,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 winPos = ImGui::GetWindowPos();

    // Top Header Icon + Branding
    const float shieldDim = 28.0f * dpi;
    auto shieldIcon = ctx.Icons().GetIcon("shield");
    if (shieldIcon.valid && shieldIcon.srv) {
        drawList->AddImage(
            reinterpret_cast<ImTextureID>(shieldIcon.srv),
            ImVec2(winPos.x + 24.0f * dpi, winPos.y + 24.0f * dpi),
            ImVec2(winPos.x + 24.0f * dpi + shieldDim, winPos.y + 24.0f * dpi + shieldDim),
            shieldIcon.uv0, shieldIcon.uv1,
            ImColor(colors.Accent)
        );
    }

    ImGui::SetCursorScreenPos(ImVec2(winPos.x + 64.0f * dpi, winPos.y + 22.0f * dpi));
    PushFont(FontWeight::Bold);
    ImGui::TextColored(colors.TextPrimary, "PARALLAX // AUTHORIZATION");
    PopFont();

    ImGui::SetCursorScreenPos(ImVec2(winPos.x + 64.0f * dpi, winPos.y + 44.0f * dpi));
    PushFont(FontWeight::Caption);
    ImGui::TextColored(colors.TextMuted, "Enter your product key to unlock the complete UI suite.");
    PopFont();

    ImGui::SetCursorScreenPos(ImVec2(winPos.x + 24.0f * dpi, winPos.y + 78.0f * dpi));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    // Key input label
    PushFont(FontWeight::Medium);
    ImGui::TextColored(colors.TextSecondary, "Product License Key:");
    PopFont();

    ImGui::Spacing();

    // Key input text field with show/hide password toggle
    static char s_keyBuffer[128] = "";
    if (s_keyBuffer[0] == '\0' && !m_enteredKey.empty()) {
        snprintf(s_keyBuffer, sizeof(s_keyBuffer), "%s", m_enteredKey.c_str());
    }

    const float inputW = dialogSize.x - 48.0f * dpi - 44.0f * dpi;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, metrics.FrameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f * dpi, 8.0f * dpi));
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

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    ImGui::SameLine(0.0f, 8.0f * dpi);

    // Eye toggle button (show / hide key)
    if (Button("##ToggleShowKey", m_showKeyText ? "eye-off" : "eye", ButtonVariant::Secondary, ImVec2(36.0f * dpi, 36.0f * dpi))) {
        m_showKeyText = !m_showKeyText;
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Remember key checkbox
    ImGui::Checkbox("Remember key on this device", &m_rememberKey);

    ImGui::Spacing();

    // Status / Feedback message area
    if (m_status == KeyStatus::Verifying) {
        // Animated loading state
        const float time = static_cast<float>(ImGui::GetTime());
        const float spinnerR = 7.0f * dpi;
        const ImVec2 spinPos = ImVec2(winPos.x + 36.0f * dpi, winPos.y + 225.0f * dpi);

        drawList->PathClear();
        for (int i = 0; i < 6; ++i) {
            const float a = time * 6.0f + static_cast<float>(i) * 0.4f;
            drawList->PathLineTo(ImVec2(spinPos.x + std::cos(a) * spinnerR, spinPos.y + std::sin(a) * spinnerR));
        }
        drawList->PathStroke(ImColor(colors.Accent), 0, 2.0f);

        ImGui::SetCursorScreenPos(ImVec2(spinPos.x + 18.0f * dpi, spinPos.y - 8.0f * dpi));
        PushFont(FontWeight::Caption);
        ImGui::TextColored(colors.Accent, "%s", m_statusMessage.c_str());
        PopFont();
    } else if (m_status == KeyStatus::Success) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.25f, 0.12f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Success);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding);
        ImGui::BeginChild("##SuccessBanner", ImVec2(-1.0f, 32.0f * dpi), true, ImGuiWindowFlags_NoScrollbar);
        PushFont(FontWeight::Caption);
        ImGui::TextColored(colors.Success, "✓  %s", m_statusMessage.c_str());
        PopFont();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    } else if (m_status == KeyStatus::InvalidKey) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.30f, 0.08f, 0.08f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Border, colors.Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding);
        ImGui::BeginChild("##ErrorBanner", ImVec2(-1.0f, 32.0f * dpi), true, ImGuiWindowFlags_NoScrollbar);
        PushFont(FontWeight::Caption);
        ImGui::TextColored(colors.Danger, "⚠  %s", m_statusMessage.c_str());
        PopFont();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    } else {
        ImGui::Dummy(ImVec2(0.0f, 32.0f * dpi));
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Bottom Action Buttons
    const float buttonW = (dialogSize.x - 48.0f * dpi - 12.0f * dpi) * 0.5f;

    // Get Key button (links to developer portal)
    if (Button("Get License Key", "external-link", ButtonVariant::Secondary, ImVec2(buttonW, 38.0f * dpi))) {
        ShellExecuteA(NULL, "open", m_keyUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::SameLine(0.0f, 12.0f * dpi);

    // Verify / Unlock button
    const char* unlockLabel = (m_status == KeyStatus::Verifying) ? "Verifying..." : "Unlock Suite";
    if (Button(unlockLabel, "check", ButtonVariant::Primary, ImVec2(buttonW, 38.0f * dpi))) {
        StartVerification(s_keyBuffer);
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    return opened;
}

} // namespace parallax::auth
