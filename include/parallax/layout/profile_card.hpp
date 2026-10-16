#pragma once

#include "parallax/core/types.hpp"
#include <imgui.h>
#include <string>

namespace parallax::layout {

class HeaderWidgets {
public:
    static bool SearchBar(std::string& inOutQuery, const char* placeholder = "Search...", float width = 0.0f);
    static void ProfileCard(const UserProfile& profile, const ImVec2& size = ImVec2(0.0f, 0.0f));
    static void WindowSizeSelector(WindowSizeTier& inOutTier);
    static void NavSideSelector(NavSide& inOutSide);
    static bool ThemeSwitcher(const char* label = nullptr);
    static bool AccentSelector(const char* label = nullptr, bool showCustomPicker = true);
};

void UserProfileCard(const UserProfile& profile, const ImVec2& size = ImVec2(0.0f, 0.0f));
bool ThemeSwitcher(const char* label = nullptr);
bool AccentSelector(const char* label = nullptr, bool showCustomPicker = true);

} // namespace parallax::layout
