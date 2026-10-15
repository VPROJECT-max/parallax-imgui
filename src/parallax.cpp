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

void SetDefaultWindowSize(float width, float height) {
    layout::WindowShell::SetDefaultSize(width, height);
}

ImVec2 GetDefaultWindowSize() {
    return layout::WindowShell::GetDefaultSize();
}

void SetWindowSizeBounds(float minWidth, float minHeight, float maxWidth, float maxHeight) {
    layout::WindowShell::SetSizeBounds(minWidth, minHeight, maxWidth, maxHeight);
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

float GetNavWidth() {
    return layout::NavBar::GetNavWidth();
}

bool IsNavExpanded() {
    return layout::NavBar::IsExpanded();
}

void SetNavExpanded(bool expanded) {
    layout::NavBar::SetExpanded(expanded);
}

void ToggleNavExpanded() {
    layout::NavBar::ToggleExpanded();
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
    layout::UserProfileCard(profile, size);
}

} // namespace parallax
