#include "parallax/parallax.hpp"
#include <imgui_internal.h>

namespace parallax {

void Init(ID3D11Device* device, ID3D11DeviceContext* context) {
    Context::Get().Initialize(device, context);
}

void Shutdown() {
    Context::Get().Shutdown();
}

void SetDpiScale(float scale) {
    Context::Get().SetDpiScale(scale);
}

float GetDpiScale() {
    return Context::Get().GetDpiScale();
}

StyleColors& Colors() {
    return StyleManager::Get().Colors();
}

StyleMetrics& Metrics() {
    return StyleManager::Get().Metrics();
}

void ApplyTheme() {
    StyleManager::Get().ApplyToImGui();
}

float AnimFloat(ImGuiID id, float target, float duration, anim::EasingPreset preset) {
    return Context::Get().Anim().AnimFloat(id, target, duration, preset);
}

ImVec4 AnimColor(ImGuiID id, const ImVec4& target, float duration, anim::EasingPreset preset) {
    return Context::Get().Anim().AnimColor(id, target, duration, preset);
}

float SpringFloat(ImGuiID id, float target, const anim::SpringConfig& config) {
    return Context::Get().Anim().SpringFloat(id, target, config);
}

ImVec2 SpringVec2(ImGuiID id, const ImVec2& target, const anim::SpringConfig& config) {
    return Context::Get().Anim().SpringVec2(id, target, config);
}

void SetSpeedMultiplier(SpeedMultiplier speed) {
    Context::Get().Anim().SetSpeedMultiplier(speed);
    Context::Get().CurrentSpeedMultiplier = speed;
}

SpeedMultiplier GetSpeedMultiplier() {
    return Context::Get().Anim().GetSpeedMultiplier();
}

void Icon(const char* name, const ImVec2& size, const ImVec4& tint) {
    Context::Get().Icons().DrawIcon(name, size, tint);
}

icons::AtlasSubTexture GetIcon(const char* name, float dpiScale, bool preserveColors) {
    return Context::Get().Icons().GetIcon(name, dpiScale, preserveColors);
}

bool IsMenuOpen() {
    return Context::Get().Input().IsMenuOpen();
}

void SetMenuOpen(bool open) {
    Context::Get().Input().SetMenuOpen(open);
}

void ToggleMenu() {
    Context::Get().Input().ToggleMenu();
}

void AddToggleKey(int vkey, bool ignoreWhenTyping) {
    Context::Get().Input().AddToggleKey(vkey, ignoreWhenTyping);
}

void ClearToggleKeys() {
    Context::Get().Input().ClearToggleKeys();
}

bool BeginWindow(const char* title, bool* p_open, ImGuiWindowFlags flags) {
    return layout::WindowShell::Begin(title, p_open, flags);
}

void EndWindow() {
    layout::WindowShell::End();
}

void SetNavSide(NavSide side) {
    Context::Get().CurrentNavSide = side;
}

NavSide GetNavSide() {
    return Context::Get().CurrentNavSide;
}

void SetWindowSizeTier(WindowSizeTier tier) {
    Context::Get().CurrentSizeTier = tier;
}

WindowSizeTier GetWindowSizeTier() {
    return Context::Get().CurrentSizeTier;
}

bool BeginNavBar() {
    return layout::NavBar::Begin(Context::Get().CurrentNavSide);
}

void EndNavBar() {
    layout::NavBar::End();
}

bool NavTab(const char* icon, const char* label, bool selected, const char* badge) {
    return layout::NavBar::TabItem(icon, label, selected, badge);
}

bool BeginSubTabs(SubTabPlacement placement) {
    return layout::SubTabs::Begin(placement);
}

void EndSubTabs() {
    layout::SubTabs::End();
}

bool SubTab(const char* label, bool selected) {
    return layout::SubTabs::Item(label, selected);
}

void BeginGrid(int columns, float spacing) {
    layout::Grid::Begin(columns, spacing);
}

void NextColumn() {
    layout::Grid::NextColumn();
}

void EndGrid() {
    layout::Grid::End();
}

bool BeginGroupbox(const char* title, const char* icon, float height) {
    return layout::Groupbox::Begin(title, icon, height);
}

void EndGroupbox() {
    layout::Groupbox::End();
}

bool SearchBar(std::string& inOutQuery, const char* placeholder, float width) {
    return layout::HeaderWidgets::SearchBar(inOutQuery, placeholder, width);
}

void UserProfileCard(const UserProfile& profile, const ImVec2& size) {
    layout::HeaderWidgets::ProfileCard(profile, size);
}

void WindowBuilder::Render(bool* p_open) {
    SetNavSide(m_navSide);
    SetWindowSizeTier(m_sizeTier);

    if (BeginWindow(m_title.c_str(), p_open)) {
        const float dpi = GetDpiScale();
        const auto& metrics = StyleManager::Get().Metrics();
        const ImVec2 winPos = ImGui::GetWindowPos();
        const ImVec2 winSize = ImGui::GetWindowSize();

        BeginNavBar();
        for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
            const auto& tab = m_tabs[i];
            const char* badge = tab.badge.empty() ? nullptr : tab.badge.c_str();
            if (NavTab(tab.icon.c_str(), tab.label.c_str(), m_selectedTab == i, badge)) {
                m_selectedTab = i;
            }
        }

        // Render profile card slot in vertical layout at bottom
        if (m_hasProfile && (m_navSide == NavSide::Left || m_navSide == NavSide::Right)) {
            const float profileY = winPos.y + winSize.y - (metrics.ProfileCardHeight + 16.0f) * dpi;
            float profileX = winPos.x + 8.0f * dpi;
            if (m_navSide == NavSide::Right) {
                profileX = winPos.x + winSize.x - (metrics.NavWidth - 8.0f) * dpi;
            }
            ImGui::SetCursorScreenPos(ImVec2(profileX, profileY));
            UserProfileCard(m_profile);
        }
        EndNavBar();

        // Calculate content area bounds based on docking side
        ImVec2 contentPos;
        ImVec2 contentSize;

        switch (m_navSide) {
            case NavSide::Left:
                contentPos = ImVec2(winPos.x + metrics.NavWidth * dpi + 16.0f * dpi, winPos.y + 16.0f * dpi);
                contentSize = ImVec2(winSize.x - metrics.NavWidth * dpi - 32.0f * dpi, winSize.y - 32.0f * dpi);
                break;
            case NavSide::Right:
                contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 16.0f * dpi);
                contentSize = ImVec2(winSize.x - metrics.NavWidth * dpi - 32.0f * dpi, winSize.y - 32.0f * dpi);
                break;
            case NavSide::Top:
                contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + metrics.NavHeight * dpi + 16.0f * dpi);
                contentSize = ImVec2(winSize.x - 32.0f * dpi, winSize.y - metrics.NavHeight * dpi - 32.0f * dpi);
                break;
            case NavSide::Bottom:
                contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 16.0f * dpi);
                contentSize = ImVec2(winSize.x - 32.0f * dpi, winSize.y - metrics.NavHeight * dpi - 32.0f * dpi);
                break;
        }

        ImGui::SetCursorScreenPos(contentPos);
        ImGui::BeginChild("##ParallaxContentRegion", contentSize, false, ImGuiWindowFlags_NoScrollbar);

        if (m_selectedTab >= 0 && m_selectedTab < static_cast<int>(m_tabs.size())) {
            TabContext tabCtx;
            if (m_tabs[m_selectedTab].callback) {
                m_tabs[m_selectedTab].callback(tabCtx);
            }
        }

        ImGui::EndChild();
        EndWindow();
    }
}

} // namespace parallax
