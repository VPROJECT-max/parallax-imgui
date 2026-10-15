#pragma once

#include <string>
#include <functional>
#include <future>
#include <memory>
#include <imgui.h>

namespace parallax::auth {

enum class KeyStatus {
    Idle,
    Verifying,
    Success,
    InvalidKey,
    ExpiredKey,
    NetworkError
};

class KeySystem {
public:
    static KeySystem& Get() noexcept;

    void SetVerifier(std::function<bool(const std::string&)> verifier);
    void SetKeyURL(const std::string& url);
    void SetRememberKey(bool remember);
    bool IsRememberKeyEnabled() const noexcept;

    void SetLicenseFilePath(const std::string& path);

    void StartVerification(const std::string& key);
    void Update();

    bool IsAuthenticated() const noexcept;
    void SetAuthenticated(bool auth) noexcept;

    KeyStatus GetStatus() const noexcept;
    const std::string& GetStatusMessage() const noexcept;
    const std::string& GetEnteredKey() const noexcept;
    void SetEnteredKey(const std::string& key);

    // Modal UI rendering
    bool RenderModal(bool* p_open = nullptr);

    // Save and load persistent license
    bool SaveRememberedKey();
    bool LoadRememberedKey();

    void Logout();

private:
    KeySystem();
    ~KeySystem() = default;

    std::function<bool(const std::string&)> m_verifier;
    std::string m_keyUrl{"https://parallax-ui.dev/license"};
    std::string m_licenseFilePath{"parallax_license.dat"};
    std::string m_enteredKey;
    std::string m_statusMessage;

    bool m_rememberKey{true};
    bool m_authenticated{false};
    bool m_showKeyText{false};
    KeyStatus m_status{KeyStatus::Idle};

    std::future<bool> m_verifyFuture;
    float m_statusTimer{0.0f};
};

// Convenience API
inline bool IsKeySystemAuthenticated() { return KeySystem::Get().IsAuthenticated(); }
inline bool RenderKeySystemModal(bool* p_open = nullptr) { return KeySystem::Get().RenderModal(p_open); }

} // namespace parallax::auth
