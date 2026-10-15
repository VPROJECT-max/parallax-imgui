#include "parallax/core/typography.hpp"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

namespace parallax {

Typography& Typography::Get() noexcept {
    static Typography instance;
    return instance;
}

std::string Typography::FindFontFile(const std::string& filename) const {
    const std::vector<std::string> searchPaths = {
        "assets/fonts/" + filename,
        "../assets/fonts/" + filename,
        "../../assets/fonts/" + filename,
        filename
    };

    for (const auto& path : searchPaths) {
        if (fs::exists(path)) {
            return path;
        }
    }

    // Check executable directory
    wchar_t exePathBuf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exePathBuf, MAX_PATH) > 0) {
        fs::path exeDir = fs::path(exePathBuf).parent_path();
        fs::path candidate = exeDir / "assets" / "fonts" / filename;
        if (fs::exists(candidate)) {
            return candidate.string();
        }
    }

    return "";
}

void Typography::Initialize(float dpiScale) {
    if (dpiScale <= 0.0f) dpiScale = 1.0f;
    m_dpiScale = dpiScale;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 2;
    config.PixelSnapH = true;
    config.RasterizerMultiply = 1.05f;

    // Resolve font file paths for Inter weights
    std::string regularPath  = FindFontFile("Inter-Regular.ttf");
    std::string mediumPath   = FindFontFile("Inter-Medium.ttf");
    std::string semiBoldPath = FindFontFile("Inter-SemiBold.ttf");
    std::string boldPath     = FindFontFile("Inter-Bold.ttf");

    // Fallback to Windows system Segoe UI if Inter is not present
    if (regularPath.empty())  regularPath  = "C:\\Windows\\Fonts\\segoeui.ttf";
    if (mediumPath.empty())   mediumPath   = regularPath;
    if (semiBoldPath.empty()) semiBoldPath = "C:\\Windows\\Fonts\\segoeuib.ttf";
    if (boldPath.empty())     boldPath     = "C:\\Windows\\Fonts\\segoeuib.ttf";

    const float szCaption  = 12.5f * dpiScale;
    const float szRegular  = 14.5f * dpiScale;
    const float szMedium   = 15.0f * dpiScale;
    const float szSemiBold = 16.5f * dpiScale;
    const float szBold     = 18.5f * dpiScale;
    const float szTitle    = 22.0f * dpiScale;

    // Load fonts
    if (fs::exists(regularPath)) {
        m_fontRegular  = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), szRegular, &config);
        m_fontCaption  = io.Fonts->AddFontFromFileTTF(regularPath.c_str(), szCaption, &config);
    }
    if (fs::exists(mediumPath)) {
        m_fontMedium   = io.Fonts->AddFontFromFileTTF(mediumPath.c_str(), szMedium, &config);
    } else {
        m_fontMedium   = m_fontRegular;
    }
    if (fs::exists(semiBoldPath)) {
        m_fontSemiBold = io.Fonts->AddFontFromFileTTF(semiBoldPath.c_str(), szSemiBold, &config);
    } else {
        m_fontSemiBold = m_fontRegular;
    }
    if (fs::exists(boldPath)) {
        m_fontBold     = io.Fonts->AddFontFromFileTTF(boldPath.c_str(), szBold, &config);
        m_fontTitle    = io.Fonts->AddFontFromFileTTF(boldPath.c_str(), szTitle, &config);
    } else {
        m_fontBold     = m_fontRegular;
        m_fontTitle    = m_fontRegular;
    }

    // Default font fallback
    if (!m_fontRegular) {
        m_fontRegular = io.Fonts->AddFontDefault();
        m_fontMedium = m_fontRegular;
        m_fontSemiBold = m_fontRegular;
        m_fontBold = m_fontRegular;
        m_fontTitle = m_fontRegular;
        m_fontCaption = m_fontRegular;
    }

    io.FontDefault = m_fontRegular;
    io.Fonts->Build();

    // Recreate ImGui DirectX 11 device objects for the new font texture
    ImGui_ImplDX11_InvalidateDeviceObjects();
    ImGui_ImplDX11_CreateDeviceObjects();

    m_initialized = true;
}

ImFont* Typography::GetFont(FontWeight weight) const noexcept {
    switch (weight) {
        case FontWeight::Caption:  return m_fontCaption ? m_fontCaption : m_fontRegular;
        case FontWeight::Medium:   return m_fontMedium ? m_fontMedium : m_fontRegular;
        case FontWeight::SemiBold: return m_fontSemiBold ? m_fontSemiBold : m_fontRegular;
        case FontWeight::Bold:     return m_fontBold ? m_fontBold : m_fontRegular;
        case FontWeight::Title:    return m_fontTitle ? m_fontTitle : m_fontBold;
        case FontWeight::Regular:
        default:                   return m_fontRegular;
    }
}

void Typography::PushFont(FontWeight weight) {
    ImFont* f = GetFont(weight);
    if (f) {
        ImGui::PushFont(f);
    } else {
        ImGui::PushFont(ImGui::GetFont());
    }
}

void Typography::PopFont() {
    ImGui::PopFont();
}

} // namespace parallax
