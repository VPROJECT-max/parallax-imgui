#pragma once

#include <imgui.h>
#include <string>
#include <vector>

namespace parallax {

enum class FontWeight {
    Regular,
    Medium,
    SemiBold,
    Bold,
    Title,
    Caption
};

class Typography {
public:
    static Typography& Get() noexcept;

    void Initialize(float dpiScale);
    bool IsInitialized() const noexcept { return m_initialized; }

    ImFont* GetFont(FontWeight weight) const noexcept;
    void PushFont(FontWeight weight);
    void PopFont();

    float GetDpiScale() const noexcept { return m_dpiScale; }

private:
    Typography() = default;
    ~Typography() = default;

    std::string FindFontFile(const std::string& filename) const;

    bool m_initialized{false};
    float m_dpiScale{1.0f};

    ImFont* m_fontRegular{nullptr};
    ImFont* m_fontMedium{nullptr};
    ImFont* m_fontSemiBold{nullptr};
    ImFont* m_fontBold{nullptr};
    ImFont* m_fontTitle{nullptr};
    ImFont* m_fontCaption{nullptr};
};

// Global convenience helpers
inline void PushFont(FontWeight weight) {
    Typography::Get().PushFont(weight);
}

inline void PopFont() {
    Typography::Get().PopFont();
}

inline ImFont* GetFont(FontWeight weight) {
    return Typography::Get().GetFont(weight);
}

} // namespace parallax
