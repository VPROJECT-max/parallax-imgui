#pragma once

#include "parallax/core/types.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/context.hpp"
#include "parallax/anim/easing.hpp"
#include "parallax/anim/spring.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/icons/icon_loader.hpp"
#include "parallax/dx11/d3d11_host.hpp"
#include "parallax/input/hotkeys.hpp"
#include "parallax/layout/window.hpp"
#include "parallax/layout/navbar.hpp"
#include "parallax/layout/tabs.hpp"
#include "parallax/layout/grid.hpp"
#include "parallax/layout/profile_card.hpp"
#include "parallax/widgets/checkbox.hpp"
#include "parallax/widgets/keybind.hpp"
#include "parallax/widgets/color_picker.hpp"
#include "parallax/widgets/dropdown.hpp"
#include "parallax/widgets/slider.hpp"
#include "parallax/widgets/collapsible_groupbox.hpp"
#include "parallax/widgets/chain.hpp"

#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace parallax {

// -----------------------------------------------------------------------------
// Core System Lifecycle
// -----------------------------------------------------------------------------
void Init(ID3D11Device* device, ID3D11DeviceContext* context);
void Shutdown();

void SetDpiScale(float scale);
float GetDpiScale();

StyleColors& Colors();
StyleMetrics& Metrics();
void ApplyTheme();

// -----------------------------------------------------------------------------
// Animation Helpers
// -----------------------------------------------------------------------------
float AnimFloat(ImGuiID id, float target, float duration = 0.25f, anim::EasingPreset preset = anim::EasingPreset::EaseOutQuint);
ImVec4 AnimColor(ImGuiID id, const ImVec4& target, float duration = 0.20f, anim::EasingPreset preset = anim::EasingPreset::EaseOutQuint);
float SpringFloat(ImGuiID id, float target, const anim::SpringConfig& config = anim::SpringConfig{});
ImVec2 SpringVec2(ImGuiID id, const ImVec2& target, const anim::SpringConfig& config = anim::SpringConfig{});

void SetSpeedMultiplier(SpeedMultiplier speed);
SpeedMultiplier GetSpeedMultiplier();

// -----------------------------------------------------------------------------
// Icon Helpers
// -----------------------------------------------------------------------------
void Icon(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f), const ImVec4& tint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
icons::AtlasSubTexture GetIcon(const char* name, float dpiScale = 1.0f, bool preserveColors = false);

// -----------------------------------------------------------------------------
// Input & Typing Safety
// -----------------------------------------------------------------------------
bool IsMenuOpen();
void SetMenuOpen(bool open);
void ToggleMenu();
void AddToggleKey(int vkey, bool ignoreWhenTyping = true);
void ClearToggleKeys();

// -----------------------------------------------------------------------------
// Immediate-Mode Layout API
// -----------------------------------------------------------------------------
bool BeginWindow(const char* title, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
void EndWindow();

void SetNavSide(NavSide side);
NavSide GetNavSide();

void SetWindowSizeTier(WindowSizeTier tier);
WindowSizeTier GetWindowSizeTier();

bool BeginNavBar();
void EndNavBar();
bool NavTab(const char* icon, const char* label, bool selected, const char* badge = nullptr);

bool BeginSubTabs(SubTabPlacement placement = SubTabPlacement::TopHorizontal);
void EndSubTabs();
bool SubTab(const char* label, bool selected);

void BeginGrid(int columns = 2, float spacing = 12.0f);
void NextColumn();
void EndGrid();

bool BeginGroupbox(const char* title, const char* icon = nullptr, float height = 0.0f);
void EndGroupbox();

bool SearchBar(std::string& inOutQuery, const char* placeholder = "Search...", float width = 0.0f);
void UserProfileCard(const UserProfile& profile, const ImVec2& size = ImVec2(0.0f, 0.0f));

// -----------------------------------------------------------------------------
// Declarative Builder API
// -----------------------------------------------------------------------------
class TabContext {
public:
    void BeginGrid(int columns = 2, float spacing = 12.0f) { parallax::BeginGrid(columns, spacing); }
    void NextColumn() { parallax::NextColumn(); }
    void EndGrid() { parallax::EndGrid(); }

    void Groupbox(const char* title, std::function<void()> content, const char* icon = nullptr, float height = 0.0f) {
        if (parallax::BeginGroupbox(title, icon, height)) {
            if (content) content();
            parallax::EndGroupbox();
        }
    }

    void CollapsibleGroupbox(const char* title, bool* p_expanded, std::function<void()> content, const char* icon = nullptr, float defaultHeight = 0.0f, bool centered = true) {
        if (parallax::BeginCollapsibleGroupbox(title, p_expanded, icon, defaultHeight, centered)) {
            if (content) content();
            parallax::EndCollapsibleGroupbox();
        }
    }

    // Phase 2 Declarative Widgets & Chaining
    ControlRow Checkbox(const char* label, bool* p_value) {
        ControlRow row(label, p_value);
        row.Style(ToggleStyle::Checkbox);
        return row;
    }

    ControlRow Toggle(const char* label, bool* p_value) {
        ControlRow row(label, p_value);
        row.Style(ToggleStyle::PillSwitch);
        return row;
    }

    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", float width = 0.0f, bool twoLine = false) {
        return parallax::SliderFloat(label, v, v_min, v_max, format, width, twoLine);
    }

    bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", float width = 0.0f, bool twoLine = false) {
        return parallax::SliderInt(label, v, v_min, v_max, format, width, twoLine);
    }

    bool RangeSlider(const char* label, float* v_min, float* v_max, float min_limit, float max_limit, const char* format = "%.2f", float width = 0.0f) {
        return parallax::RangeSliderFloat(label, v_min, v_max, min_limit, max_limit, format, width);
    }

    bool Dropdown(const char* label, const std::vector<std::string>& items, int* selected_index, const std::vector<std::string>& item_icons = {}, float width = 0.0f) {
        return parallax::Dropdown(label, items, selected_index, item_icons, width);
    }

    bool MultiDropdown(const char* label, const std::vector<std::string>& items, std::vector<bool>& selected_states, const std::vector<std::string>& item_icons = {}, float width = 0.0f) {
        return parallax::MultiDropdown(label, items, selected_states, item_icons, width);
    }

    bool Color(const char* label, ImVec4* color, bool hasAlpha = true) {
        return parallax::ColorPicker(label, color, hasAlpha);
    }

    bool Keybind(const char* label, parallax::Keybind* keybind) {
        return parallax::KeybindWidget(label, keybind);
    }
};

inline ControlRow Row(const char* label, bool* p_value) {
    return ControlRow(label, p_value);
}

using TabRenderCallback = std::function<void(TabContext& ctx)>;

struct BuilderTab {
    std::string icon;
    std::string label;
    std::string badge;
    TabRenderCallback callback;
};

class WindowBuilder {
public:
    explicit WindowBuilder(std::string title) : m_title(std::move(title)) {}

    WindowBuilder& WithNavSide(NavSide side) noexcept {
        m_navSide = side;
        return *this;
    }

    WindowBuilder& WithSizeTier(WindowSizeTier tier) noexcept {
        m_sizeTier = tier;
        return *this;
    }

    WindowBuilder& WithUserProfile(UserProfile profile) noexcept {
        m_profile = std::move(profile);
        m_hasProfile = true;
        return *this;
    }

    WindowBuilder& AddTab(std::string icon, std::string label, TabRenderCallback callback, std::string badge = "") {
        m_tabs.push_back({std::move(icon), std::move(label), std::move(badge), std::move(callback)});
        return *this;
    }

    void Render(bool* p_open = nullptr);

private:
    std::string m_title;
    NavSide m_navSide{NavSide::Left};
    WindowSizeTier m_sizeTier{WindowSizeTier::Standard};
    UserProfile m_profile;
    bool m_hasProfile{false};
    std::vector<BuilderTab> m_tabs;
    int m_selectedTab{0};
};

inline WindowBuilder Window(std::string title) {
    return WindowBuilder(std::move(title));
}

} // namespace parallax
