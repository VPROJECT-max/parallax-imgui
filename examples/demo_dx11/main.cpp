#include "parallax/parallax.hpp"
#include <imgui_internal.h>
#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

int RunDemo(HINSTANCE hInstance) {
    // 1. Initialize standalone DirectX 11 Host window
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1200, 750, L"Parallax UI v" L"1.1.0 — C++20 DirectX 11 Interface Framework")) {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 Host Window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2. Initialize Parallax UI Library (Loads Inter Fonts, Design Tokens & SVG Atlas)
    parallax::Init(host.GetDevice(), host.GetContext());

    // Initial welcome notification
    parallax::Notify(
        parallax::NotificationLevel::Success,
        "Parallax v" PARALLAX_VERSION_STRING " Ready",
        "Graphite & Porcelain themes, Muted Sage accent engine, and redesigned components initialized.",
        4.5f
    );

    // 3. Demo application state
    bool menuOpen = true;
    int activeMainTab = 0;
    int activeLayoutSubTab = 0;
    std::string searchQuery = "";

    // Text inputs state
    std::string testSearchInput = "";
    std::string testUsernameInput = "alex.mercer";
    std::string testApiToken = "prlx-live-94a82f0c11d";

    // Recent activity log for live interaction feedback
    std::deque<std::string> activityLog;
    auto logEvent = [&](const std::string& msg) {
        activityLog.push_front(msg);
        if (activityLog.size() > 6) activityLog.pop_back();
    };
    logEvent("Parallax v" PARALLAX_VERSION_STRING " initialized (1137x699 fixed shell).");

    // Component Showcase State: Toggles, Checkboxes & Chained Rows
    bool boxEsp = true;
    bool snaplines = false;
    bool skeletons = true;
    bool glowEffect = true;
    bool soundAlerts = true;
    bool compactToggleA = true;
    bool compactToggleB = false;
    bool indeterminateCheck = false;
    bool isIndeterminateMode = true;
    bool disabledDemo = false;

    // Keybind State
    parallax::Keybind boxKey{ 'B', parallax::KeybindMode::Toggle, false };
    parallax::Keybind aimKey{ VK_XBUTTON1, parallax::KeybindMode::Hold, false };
    parallax::Keybind triggerKey{ VK_SPACE, parallax::KeybindMode::AlwaysOn, true };
    parallax::Keybind glowKey{ 'G', parallax::KeybindMode::Toggle, false };

    // Color Picker State (Restrained, cohesive default palette)
    ImVec4 boxColor{ 0.471f, 0.631f, 0.545f, 1.0f };      // Muted Sage #78A18B
    ImVec4 snaplineColor{ 0.769f, 0.541f, 0.416f, 0.90f }; // Desaturated Copper #C48A6A
    ImVec4 skeletonColor{ 0.880f, 0.890f, 0.910f, 1.0f };  // Soft Off-White
    ImVec4 glowColor{ 0.431f, 0.569f, 0.690f, 0.90f };     // Subtle Steel Blue #6E91B0

    // Dropdown State
    std::vector<std::string> hitboxItems = { "Head Target", "Neck / Spine", "Upper Torso", "Lower Pelvis", "Closest Bone" };
    std::vector<std::string> hitboxIcons = { "target", "crosshair", "shield", "circle", "zap" };
    int selectedHitbox = 0;

    std::vector<std::string> filterAlgorithmItems = { "Kalman Predictive", "Hermite Cubic", "Spring Damper", "Linear Direct" };
    std::vector<std::string> filterAlgorithmIcons = { "cpu", "activity", "sliders", "zap" };
    int selectedAlgorithm = 2;

    std::vector<std::string> visualElements = { "Health Gauge", "Shield Bar", "Distance Tag", "Active Weapon", "Operator Tag", "Offscreen Compass" };
    std::vector<std::string> visualIcons = { "heart", "shield", "map-pin", "zap", "user", "navigation" };
    std::vector<bool> selectedElements = { true, true, true, false, true, false };

    // Sliders & Dual Range Sliders
    float fovValue = 90.0f;
    float smoothingValue = 4.5f;
    float exposureStop = 0.0f;
    int maxTargetCount = 6;
    float renderDistanceMin = 25.0f;
    float renderDistanceMax = 320.0f;
    int pingRangeMin = 20;
    int pingRangeMax = 115;

    // Collapsible Groupbox States
    bool visualGroupExpanded = true;
    bool audioGroupExpanded = true;
    bool advancedGroupExpanded = true;

    // Bind variables to JSON Configuration Engine
    auto& cfg = parallax::config::ConfigManager::Get();
    cfg.BindBool("visuals.box_esp", &boxEsp, true);
    cfg.BindBool("visuals.snaplines", &snaplines, false);
    cfg.BindBool("visuals.skeletons", &skeletons, true);
    cfg.BindBool("visuals.glow_effect", &glowEffect, true);
    cfg.BindBool("audio.sound_alerts", &soundAlerts, true);

    cfg.BindColor("visuals.box_color", &boxColor, ImVec4(0.471f, 0.631f, 0.545f, 1.0f));
    cfg.BindColor("visuals.snapline_color", &snaplineColor, ImVec4(0.769f, 0.541f, 0.416f, 0.90f));
    cfg.BindColor("visuals.skeleton_color", &skeletonColor, ImVec4(0.880f, 0.890f, 0.910f, 1.0f));
    cfg.BindColor("visuals.glow_color", &glowColor, ImVec4(0.431f, 0.569f, 0.690f, 0.90f));

    cfg.BindKeybind("keys.box_key", &boxKey);
    cfg.BindKeybind("keys.aim_key", &aimKey);
    cfg.BindKeybind("keys.trigger_key", &triggerKey);
    cfg.BindKeybind("keys.glow_key", &glowKey);

    cfg.BindFloat("aim.fov", &fovValue, 90.0f);
    cfg.BindFloat("aim.smoothing", &smoothingValue, 4.5f);
    cfg.BindInt("aim.max_targets", &maxTargetCount, 6);
    cfg.BindFloat("render.distance_min", &renderDistanceMin, 25.0f);
    cfg.BindFloat("render.distance_max", &renderDistanceMax, 320.0f);
    cfg.BindInt("network.ping_min", &pingRangeMin, 20);
    cfg.BindInt("network.ping_max", &pingRangeMax, 115);

    cfg.BindString("user.username", &testUsernameInput, "alex.mercer");
    cfg.BindString("user.api_token", &testApiToken, "prlx-live-94a82f0c11d");
    cfg.BindMultiBool("visuals.indicators", &selectedElements);

    cfg.BindBool("groups.visuals_expanded", &visualGroupExpanded, true);
    cfg.BindBool("groups.audio_expanded", &audioGroupExpanded, true);
    bool showKeyModal = false;

    // Search catalog across tabs and sections for global navigation
    struct SearchCatalogItem {
        int tabIndex;
        const char* tabName;
        const char* sectionName;
        const char* itemName;
        const char* description;
        const char* icon;
        bool* expandFlag;
    };

    const SearchCatalogItem searchCatalog[] = {
        // Tab 0: Components
        { 0, "Components", "Toggles, Checkboxes & Chained Rows", "Bounding Box Overlay", "Render 2D/3D spatial bounds with adaptive contrast", "eye", nullptr },
        { 0, "Components", "Toggles, Checkboxes & Chained Rows", "Trajectory Snaplines", "Trace line projections from viewport origin", "navigation", nullptr },
        { 0, "Components", "Toggles, Checkboxes & Chained Rows", "Bone Skeletons", "Bone matrix kinematic wireframe overlay", "activity", nullptr },
        { 0, "Components", "Toggles, Checkboxes & Chained Rows", "Rim Glow Effect", "Fresnel peripheral luminescence shader", "sun", nullptr },
        { 0, "Components", "Interactive Keybind Controls", "Bounding Box Hotkey", "Hardware keybind for Bounding Box Overlay", "lock", nullptr },
        { 0, "Components", "Interactive Keybind Controls", "Aim Assist Keybind", "Hardware keybind for Aim Smoothing Assist", "crosshair", nullptr },
        { 0, "Components", "Interactive Keybind Controls", "Triggerbot Activation", "Hardware keybind for Automatic Fire Trigger", "zap", nullptr },
        { 0, "Components", "Interactive Keybind Controls", "Rim Glow Keybind", "Hardware keybind for Peripheral Glow Shader", "sun", nullptr },
        { 0, "Components", "Precision Sliders & Dual Range Sliders", "Aim Assist FOV", "Field of view targeting cone angle in degrees", "sliders", nullptr },
        { 0, "Components", "Precision Sliders & Dual Range Sliders", "Smoothing Factor", "Exponential mouse smoothing coefficient", "sliders", nullptr },
        { 0, "Components", "Precision Sliders & Dual Range Sliders", "Max Target Count", "Maximum simultaneous tracked targets", "layers", nullptr },
        { 0, "Components", "Precision Sliders & Dual Range Sliders", "Render Distance Range", "Near and far render distance limits in meters", "sliders", nullptr },
        { 0, "Components", "Precision Sliders & Dual Range Sliders", "Ping Compensation Range", "Network round-trip latency compensation bounds", "sliders", nullptr },
        { 0, "Components", "Button Family & Variants", "Deploy Module Button", "Accent-colored primary button with tactile feedback", "zap", nullptr },
        { 0, "Components", "Button Family & Variants", "Sync Profile Button", "Bordered secondary action button", "refresh-cw", nullptr },
        { 0, "Components", "Button Family & Variants", "Purge Cache Button", "High-visibility terracotta button for irreversible actions", "trash-2", nullptr },
        // Tab 1: Selectors & Inputs
        { 1, "Selectors", "Dropdowns & Multi-Select Menus", "Target Bone Priority", "Single selection dropdown selector", "layers", nullptr },
        { 1, "Selectors", "Dropdowns & Multi-Select Menus", "Active Telemetry Layers", "Multi-selection combobox with inline count badge", "check", nullptr },
        { 1, "Selectors", "Color Picker & Swatch Controls", "Primary Bounding Color", "Color picker swatch with HSV canvas", "palette", nullptr },
        { 1, "Selectors", "Text Inputs & Password Masking", "Operator Identifier", "Single line text input with clear button", "user", nullptr },
        { 1, "Selectors", "Text Inputs & Password Masking", "Secret API Token", "Masked password input with eye visibility toggle", "lock", nullptr },
        { 1, "Selectors", "Interactive Keybind Controls", "Keybind Management", "Configure Hold, Toggle, or Always-On hardware keybinds", "cpu", nullptr },
        // Tab 2: Appearance
        { 2, "Appearance", "Theme Engine — AMOLED, Graphite & Porcelain", "Theme Switcher", "AMOLED Black, Graphite Dark, and Porcelain Light", "sun", nullptr },
        { 2, "Appearance", "Theme Engine — AMOLED, Graphite & Porcelain", "Accent Color Presets", "Parallax Violet (#9998F6), Sage, Monochrome, Copper, Steel Blue", "palette", nullptr },
        { 2, "Appearance", "Theme Engine — AMOLED, Graphite & Porcelain", "Custom Accent Picker", "RGB color picker for bespoke accent color", "sliders", nullptr },
        { 2, "Appearance", "Theme Engine — AMOLED, Graphite & Porcelain", "Animation Speed Multiplier", "Speed scaling from Fast (1.5x) to Off (Instant snap)", "clock", nullptr },
        { 2, "Appearance", "Toast Notification System", "Toast Notifications", "Success, info, warning, and danger toast notifications", "bell", nullptr },
        // Tab 3: Layouts
        { 3, "Layouts", "Navigation Presets & Placement", "Modern Sidebar Preset", "Expanded 200px Left Dock", "sidebar", nullptr },
        { 3, "Layouts", "Navigation Presets & Placement", "Compact Tactical Preset", "Collapsed 68px Icon Dock", "crosshair", nullptr },
        { 3, "Layouts", "Navigation Presets & Placement", "Minimal Tabbed Preset", "Top horizontal navigation bar", "maximize-2", nullptr },
        { 3, "Layouts", "Navigation Presets & Placement", "Navigation Dock Side", "Dock sidebar to Left, Right, Top, or Bottom", "layout", nullptr },
        { 3, "Layouts", "Collapsible Trajectory Solver", "Sub-frame Kinematics", "Collapsible trajectory solver kinematics row", "cpu", &advancedGroupExpanded },
        { 3, "Layouts", "Collapsible Trajectory Solver", "Solver Tick Rate", "Collapsible solver tick rate slider", "sliders", &advancedGroupExpanded },
        { 3, "Layouts", "Fixed Menu Shell & Automatic Card Sizing", "Fixed Menu Shell", "1137x699 fixed footprint with auto-fitting cards", "maximize", nullptr },
        { 3, "Layouts", "Compact Card A (2 Controls)", "Auto-Sizing Cards", "Compact cards that auto-fit content height", "layers", nullptr },
        // Tab 4: Icons
        { 4, "Icons", "On-Demand 2,077 SVG Icon Atlas", "SVG Icon Atlas", "High-DPI rasterized SVG icon catalog", "image", nullptr },
        // Tab 5: Configs
        { 5, "Configs", "Profile Management", "Configuration Profiles", "Save, load, import, export, and delete JSON configs", "database", nullptr },
        // Tab 6: Key System
        { 6, "Key System", "Developer License Status", "License Key Status", "Asynchronous seat validation and modal dialog", "shield", nullptr },
        { 6, "Key System", "Authorization Simulator", "Key Verification Simulator", "Test non-blocking authentication with VIP keys", "terminal", nullptr }
    };

    std::string spotlightSection = "";
    float spotlightTimer = 0.0f;

    // 4. Main message and render loop
    host.RunMainLoop([&]() {
        if (spotlightTimer > 0.0f) {
            spotlightTimer -= ImGui::GetIO().DeltaTime;
            if (spotlightTimer < 0.0f) spotlightTimer = 0.0f;
        }
        // Update live keybind states
        parallax::UpdateKeybindState(boxKey);
        parallax::UpdateKeybindState(aimKey);
        parallax::UpdateKeybindState(triggerKey);
        parallax::UpdateKeybindState(glowKey);

        // Update auto-save and key system async validation
        cfg.Update(ImGui::GetIO().DeltaTime);
        parallax::auth::KeySystem::Get().Update();

        // Always render floating watermark status pill
        parallax::Watermark("PARALLAX v" PARALLAX_VERSION_STRING, "Alex Mercer", true, true);

        // Always render toast notifications
        parallax::RenderNotifications();

        // Optional Key System Modal popup
        if (showKeyModal) {
            parallax::auth::KeySystem::Get().RenderModal(&showKeyModal);
        }

        // Draw guidance text when menu is closed
        if (!menuOpen) {
            ImGui::SetNextWindowPos(ImVec2(30.0f, 60.0f));
            ImGui::Begin("##ParallaxBackgroundHint", nullptr,
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
            parallax::PushFont(parallax::FontWeight::SemiBold);
            ImGui::TextColored(parallax::Colors().TextSecondary, "Press [Right Shift] or [Insert] to open Parallax v" PARALLAX_VERSION_STRING);
            parallax::PopFont();
            ImGui::End();
        }

        // Render Parallax Window (Strictly fixed 1137x699 logical dimensions)
        if (parallax::BeginWindow("Parallax UI Architecture", &menuOpen)) {
            const float dpi = parallax::GetDpiScale();
            const auto& metrics = parallax::Metrics();
            const auto& colors = parallax::Colors();
            const ImVec2 winPos = ImGui::GetWindowPos();
            const ImVec2 winSize = ImGui::GetWindowSize();
            const auto navSide = parallax::GetNavSide();

            // -----------------------------------------------------------------
            // Navigation Surface (Sidebar / Top / Bottom)
            // -----------------------------------------------------------------
            parallax::BeginNavBar();
            if (parallax::NavTab("sliders",  "Components",    activeMainTab == 0, "v1.1")) activeMainTab = 0;
            if (parallax::NavTab("layers",   "Selectors",     activeMainTab == 1))         activeMainTab = 1;
            if (parallax::NavTab("palette",  "Appearance",    activeMainTab == 2, "NEW"))  activeMainTab = 2;
            if (parallax::NavTab("layout",   "Layouts",       activeMainTab == 3))         activeMainTab = 3;
            if (parallax::NavTab("image",    "Icons",         activeMainTab == 4))         activeMainTab = 4;
            if (parallax::NavTab("database", "Configs",       activeMainTab == 5))         activeMainTab = 5;
            if (parallax::NavTab("shield",   "Key System",    activeMainTab == 6))         activeMainTab = 6;

            // User Profile Card anchored at bottom of vertical sidebars
            if (navSide == parallax::NavSide::Left || navSide == parallax::NavSide::Right) {
                const float curNavW = parallax::GetNavWidth() * dpi;
                const float profileCardW = curNavW - 16.0f * dpi;
                const float profileY = winPos.y + winSize.y - (metrics.ProfileCardHeight + 12.0f) * dpi;
                float profileX = (navSide == parallax::NavSide::Right)
                    ? (winPos.x + winSize.x - curNavW + 8.0f * dpi)
                    : (winPos.x + 8.0f * dpi);
                ImGui::SetCursorScreenPos(ImVec2(profileX, profileY));
                parallax::UserProfileCard(parallax::Context::Get().Profile, ImVec2(profileCardW, metrics.ProfileCardHeight * dpi));
            }
            parallax::EndNavBar();

            // -----------------------------------------------------------------
            // Workspace Surface Region Bounds
            // -----------------------------------------------------------------
            const float dynamicNavW = parallax::GetNavWidth() * dpi;
            ImVec2 contentPos;
            ImVec2 contentSize;

            switch (navSide) {
                case parallax::NavSide::Left:
                    contentPos = ImVec2(winPos.x + dynamicNavW + 16.0f * dpi, winPos.y + 14.0f * dpi);
                    contentSize = ImVec2(winSize.x - dynamicNavW - 32.0f * dpi, winSize.y - 28.0f * dpi);
                    break;
                case parallax::NavSide::Right:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 14.0f * dpi);
                    contentSize = ImVec2(winSize.x - dynamicNavW - 32.0f * dpi, winSize.y - 28.0f * dpi);
                    break;
                case parallax::NavSide::Top:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + metrics.NavHeight * dpi + 14.0f * dpi);
                    contentSize = ImVec2(winSize.x - 32.0f * dpi, winSize.y - metrics.NavHeight * dpi - 28.0f * dpi);
                    break;
                case parallax::NavSide::Bottom:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 14.0f * dpi);
                    contentSize = ImVec2(winSize.x - 32.0f * dpi, winSize.y - metrics.NavHeight * dpi - 28.0f * dpi);
                    break;
            }

            auto SpotlightCheck = [&](const char* title) {
                if (spotlightTimer > 0.0f && spotlightSection == title) {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    float pulse = (std::sin(spotlightTimer * 8.0f) * 0.5f + 0.5f);
                    ImVec4 pulseCol = colors.Accent;
                    pulseCol.w = 0.45f + 0.55f * pulse;
                    const ImVec2 rMin = ImGui::GetItemRectMin();
                    const ImVec2 rMax = ImGui::GetItemRectMax();
                    dl->AddRect(
                        ImVec2(rMin.x - 2.0f * dpi, rMin.y - 2.0f * dpi),
                        ImVec2(rMax.x + 2.0f * dpi, rMax.y + 2.0f * dpi),
                        ImColor(pulseCol),
                        metrics.FrameRounding * dpi,
                        0,
                        2.0f * dpi
                    );
                }
            };

            auto RenderSearchPopover = [&](std::string& query, const ImVec2& searchMin, const ImVec2& searchMax) {
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    query.clear();
                    return;
                }

                std::string q = query;
                while (!q.empty() && std::isspace(static_cast<unsigned char>(q.front()))) q.erase(q.begin());
                while (!q.empty() && std::isspace(static_cast<unsigned char>(q.back()))) q.pop_back();
                if (q.empty()) return;

                std::vector<std::string> tokens;
                {
                    std::string cur;
                    for (char ch : q) {
                        if (std::isspace(static_cast<unsigned char>(ch))) {
                            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
                        } else {
                            cur.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
                        }
                    }
                    if (!cur.empty()) tokens.push_back(cur);
                }

                auto toLower = [](std::string_view s) {
                    std::string out;
                    out.reserve(s.size());
                    for (char c : s) out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                    return out;
                };

                std::vector<SearchCatalogItem> matches;
                for (const auto& item : searchCatalog) {
                    std::string hay = toLower(item.itemName) + " " + toLower(item.description) + " " +
                                      toLower(item.sectionName) + " " + toLower(item.tabName);
                    bool allFound = true;
                    for (const auto& tok : tokens) {
                        if (hay.find(tok) == std::string::npos) {
                            allFound = false;
                            break;
                        }
                    }
                    if (allFound) {
                        matches.push_back(item);
                    }
                }

                ImGui::SetNextWindowPos(ImVec2(searchMin.x, searchMax.y + 6.0f * dpi));
                ImGui::SetNextWindowSize(ImVec2(390.0f * dpi, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi, 8.0f * dpi));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, metrics.PopupRounding * dpi);
                ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_PopupBg, colors.PopupBg);
                ImGui::PushStyleColor(ImGuiCol_Border, colors.BorderStrong);

                if (ImGui::Begin("##ParallaxSearchPopover", nullptr,
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing))
                {
                    if (matches.empty()) {
                        ImGui::Spacing();
                        parallax::PushFont(parallax::FontWeight::Medium);
                        ImGui::TextColored(colors.TextMuted, "No matching controls found for \"%s\"", q.c_str());
                        parallax::PopFont();
                        parallax::PushFont(parallax::FontWeight::Caption);
                        ImGui::TextColored(colors.TextMuted, "Press Esc to clear search");
                        parallax::PopFont();
                        ImGui::Spacing();
                    } else {
                        int count = std::min(static_cast<int>(matches.size()), 6);
                        for (int i = 0; i < count; ++i) {
                            const auto& item = matches[i];
                            ImGui::PushID(i);
                            const ImVec2 rowPos = ImGui::GetCursorScreenPos();
                            const float rowW = ImGui::GetContentRegionAvail().x;
                            const float rowH = 42.0f * dpi;
                            const ImRect rowBb(rowPos, ImVec2(rowPos.x + rowW, rowPos.y + rowH));

                            ImGui::ItemSize(rowBb);
                            if (ImGui::ItemAdd(rowBb, ImGui::GetID("##SearchRow"))) {
                                bool hovered = false, held = false;
                                bool pressed = ImGui::ButtonBehavior(rowBb, ImGui::GetID("##SearchRow"), &hovered, &held);

                                ImDrawList* dl = ImGui::GetWindowDrawList();
                                if (hovered) {
                                    dl->AddRectFilled(rowBb.Min, rowBb.Max, ImColor(colors.WidgetHover), metrics.FrameRounding * dpi);
                                    dl->AddRect(rowBb.Min, rowBb.Max, ImColor(colors.AccentMuted), metrics.FrameRounding * dpi);
                                }

                                const float iconSz = 16.0f * dpi;
                                const ImVec2 icPos(rowBb.Min.x + 8.0f * dpi, rowBb.Min.y + (rowH - iconSz) * 0.5f);
                                parallax::Context::Get().Icons().DrawIcon(dl, item.icon, icPos, ImVec2(iconSz, iconSz), hovered ? colors.Accent : colors.TextSecondary);

                                const float textX = icPos.x + iconSz + 10.0f * dpi;
                                parallax::PushFont(parallax::FontWeight::SemiBold);
                                dl->AddText(ImVec2(textX, rowBb.Min.y + 4.0f * dpi), ImColor(hovered ? colors.TextPrimary : colors.TextSecondary), item.itemName);
                                parallax::PopFont();

                                parallax::PushFont(parallax::FontWeight::Caption);
                                char subBuf[128];
                                snprintf(subBuf, sizeof(subBuf), "%s > %s", item.tabName, item.sectionName);
                                dl->AddText(ImVec2(textX, rowBb.Min.y + 22.0f * dpi), ImColor(colors.TextMuted), subBuf);
                                parallax::PopFont();

                                parallax::PushFont(parallax::FontWeight::Caption);
                                const ImVec2 badgeTextSz = ImGui::CalcTextSize(item.tabName);
                                const float badgePad = 6.0f * dpi;
                                const float badgeW = badgeTextSz.x + badgePad * 2.0f;
                                const float badgeH = 18.0f * dpi;
                                const ImVec2 badgeMin(rowBb.Max.x - badgeW - 8.0f * dpi, rowBb.Min.y + (rowH - badgeH) * 0.5f);
                                const ImVec2 badgeMax(badgeMin.x + badgeW, badgeMin.y + badgeH);
                                dl->AddRectFilled(badgeMin, badgeMax, ImColor(colors.AccentMuted), 4.0f * dpi);
                                dl->AddText(ImVec2(badgeMin.x + badgePad, badgeMin.y + (badgeH - badgeTextSz.y) * 0.5f), ImColor(colors.Accent), item.tabName);
                                parallax::PopFont();

                                if (pressed) {
                                    activeMainTab = item.tabIndex;
                                    if (item.expandFlag) {
                                        *item.expandFlag = true;
                                    }
                                    spotlightSection = item.sectionName;
                                    spotlightTimer = 2.5f;
                                    query.clear();
                                }
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::End();
                }
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(3);
            };

            // -----------------------------------------------------------------
            // Pinned Workspace Header Bar (Never scrolls with content)
            // -----------------------------------------------------------------
            const float headerH = 36.0f * dpi;
            ImGui::SetCursorScreenPos(contentPos);
            ImGui::BeginGroup();
            {
                const char* sectionTitles[] = {
                    "Component Showcase",
                    "Selectors & Inputs",
                    "Themes & Appearance",
                    "Layout Architecture",
                    "SVG Icon Atlas",
                    "Configuration Engine",
                    "Key Authorization"
                };
                const int safeTab = std::clamp(activeMainTab, 0, 6);

                parallax::PushFont(parallax::FontWeight::Bold);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f * dpi);
                ImGui::TextColored(colors.TextPrimary, "%s", sectionTitles[safeTab]);
                parallax::PopFont();

                ImGui::SameLine(220.0f * dpi);
                const ImVec2 searchPosBefore = ImGui::GetCursorScreenPos();
                parallax::SearchBar(searchQuery, "Search controls or features...", 240.0f * dpi);
                const ImVec2 searchPosAfter = ImGui::GetItemRectMax();

                // Quick Theme Toggle Button (cycles Amoled -> Dark -> Light)
                const float rightGroupW = 240.0f * dpi;
                const float availX = ImGui::GetContentRegionAvail().x;
                if (availX > rightGroupW) {
                    ImGui::SameLine(ImGui::GetCursorPosX() + availX - rightGroupW);
                } else {
                    ImGui::SameLine();
                }

                auto curTheme = parallax::StyleManager::Get().GetTheme();
                const char* themeLabel = "AMOLED Black";
                const char* themeIcon = "circle";
                if (curTheme == parallax::ThemeMode::Dark) {
                    themeLabel = "Graphite (Dark)";
                    themeIcon = "moon";
                } else if (curTheme == parallax::ThemeMode::Light) {
                    themeLabel = "Porcelain (Light)";
                    themeIcon = "sun";
                }

                if (parallax::ButtonCompact(themeLabel, themeIcon, parallax::ButtonVariant::Secondary)) {
                    parallax::ToggleTheme();
                }
                ImGui::SameLine(0.0f, 8.0f * dpi);
                parallax::PushFont(parallax::FontWeight::Caption);
                ImGui::TextColored(colors.TextMuted, "1137x699");
                parallax::PopFont();

                if (!searchQuery.empty()) {
                    RenderSearchPopover(searchQuery, searchPosBefore, searchPosAfter);
                }
            }
            ImGui::EndGroup();

            // 1px Subtle divider line between Header and Content
            const float dividerY = contentPos.y + headerH + 4.0f * dpi;
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(contentPos.x, dividerY),
                ImVec2(contentPos.x + contentSize.x, dividerY),
                ImColor(colors.BorderSubtle)
            );

            // -----------------------------------------------------------------
            // Dedicated Scrollable Tab Region
            // -----------------------------------------------------------------
            const ImVec2 scrollRegionPos(contentPos.x, dividerY + 6.0f * dpi);
            const ImVec2 scrollRegionSize(contentSize.x, contentSize.y - (dividerY - contentPos.y) - 6.0f * dpi);

            static float s_targetScrollY = 0.0f;
            static float s_currentScrollY = 0.0f;
            static float s_scrollVelocity = 0.0f;
            static int s_prevTab = -1;

            if (s_prevTab != activeMainTab) {
                s_prevTab = activeMainTab;
                s_targetScrollY = 0.0f;
                s_currentScrollY = 0.0f;
                s_scrollVelocity = 0.0f;
            }

            ImGui::SetCursorScreenPos(scrollRegionPos);
            ImGui::SetNextWindowScroll(ImVec2(0.0f, std::round(s_currentScrollY)));
            ImGui::BeginChild("##ParallaxTabScrollRegion", scrollRegionSize, false, ImGuiWindowFlags_NoScrollWithMouse);

            ImGuiWindow* scrollWin = ImGui::GetCurrentWindow();
            bool isDraggingScrollbar = (ImGui::GetActiveID() == ImGui::GetWindowScrollbarID(scrollWin, ImGuiAxis_Y));
            if (isDraggingScrollbar) {
                s_targetScrollY = ImGui::GetScrollY();
                s_currentScrollY = s_targetScrollY;
                s_scrollVelocity = 0.0f;
            } else {
                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId)) {
                    float wheel = ImGui::GetIO().MouseWheel;
                    if (wheel != 0.0f) {
                        s_scrollVelocity -= wheel * 360.0f * dpi;
                    }
                }

                float dt = ImGui::GetIO().DeltaTime;
                if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;

                if (std::abs(s_scrollVelocity) > 0.1f) {
                    s_targetScrollY += s_scrollVelocity * dt;
                    s_scrollVelocity *= std::exp(-14.0f * dt);
                } else {
                    s_scrollVelocity = 0.0f;
                }

                const float maxScroll = ImGui::GetScrollMaxY();
                s_targetScrollY = std::clamp(s_targetScrollY, 0.0f, maxScroll);
                s_currentScrollY = std::lerp(s_currentScrollY, s_targetScrollY, 1.0f - std::exp(-22.0f * dt));
                if (std::abs(s_currentScrollY - s_targetScrollY) < 0.3f) {
                    s_currentScrollY = s_targetScrollY;
                }
            }

            // =================================================================
            // TAB 0: COMPONENT SHOWCASE
            // (Toggles, Checkboxes, Chained Rows, Sliders, Dual Range Sliders, Buttons)
            // =================================================================
            if (activeMainTab == 0) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Toggles, Checkboxes & Chained Rows
                parallax::BeginGroupbox("Toggles, Checkboxes & Chained Rows", "sliders");
                parallax::GroupboxDescription("Custom kinetic switch tracks, center-expanding checkboxes, and multi-attachment rows.");

                parallax::Row("Bounding Box Overlay", &boxEsp)
                    .Icon("eye")
                    .Description("Render 2D/3D spatial bounds with adaptive contrast")
                    .Tooltip("Supports inline color swatch and hardware keybind")
                    .Keybind(&boxKey)
                    .Color(&boxColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Bounding Box: ") + (val ? "ON" : "OFF"));
                    });

                parallax::Row("Trajectory Snaplines", &snaplines)
                    .Style(parallax::ToggleStyle::Checkbox)
                    .Icon("navigation")
                    .Description("Draw precision vector line toward priority target")
                    .Color(&snaplineColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Snaplines: ") + (val ? "ON" : "OFF"));
                    });

                parallax::Row("Skeletal Joint Mesh", &skeletons)
                    .Icon("activity")
                    .Description("Overlay articulated joint hierarchy")
                    .Color(&skeletonColor);

                parallax::Row("Surface Rim Illumination", &glowEffect)
                    .Icon("zap")
                    .Description("Restrained edge contour shader pass")
                    .Keybind(&glowKey)
                    .Color(&glowColor);

                parallax::Row("Partial Override Group", &indeterminateCheck)
                    .Style(parallax::ToggleStyle::Checkbox)
                    .Indeterminate(isIndeterminateMode)
                    .Icon("layers")
                    .Description("Demonstrates indeterminate horizontal dash state")
                    .OnChange([&](bool) {
                        isIndeterminateMode = false;
                    });

                parallax::Row("Compact Telemetry Feed", &compactToggleA)
                    .Compact(true)
                    .Icon("wifi");

                parallax::Row("Hardware Raytracing Pass (Locked)", &disabledDemo)
                    .Icon("lock")
                    .Description("Requires DirectX 12 Ultimate device")
                    .Disabled(true);

                parallax::EndGroupbox();
                SpotlightCheck("Toggles, Checkboxes & Chained Rows");

                // Second auto-sized card in Column 1: Button Family
                parallax::BeginGroupbox("Button Family & Variants", "zap");
                parallax::GroupboxDescription("Primary, Secondary, Subtle/Ghost, Destructive, Compact, and Icon-Only buttons.");

                const float halfW = (ImGui::GetContentRegionAvail().x - 8.0f * dpi) * 0.5f;
                if (parallax::Button("Deploy Module", "shield", parallax::ButtonVariant::Primary, ImVec2(halfW, 0.0f))) {
                    parallax::Notify(parallax::NotificationLevel::Success, "Module Deployed", "All telemetry and verification hooks active.", 3.5f);
                    logEvent("Deployed primary module");
                }
                ImGui::SameLine(0.0f, 8.0f * dpi);
                if (parallax::Button("Sync Profile", "refresh-cw", parallax::ButtonVariant::Secondary, ImVec2(halfW, 0.0f))) {
                    parallax::Notify(parallax::NotificationLevel::Info, "Synchronized", "Workspace state synced with active profile.", 3.0f);
                    logEvent("Synced profile");
                }

                ImGui::Spacing();

                if (parallax::Button("Inspect Logs", "terminal", parallax::ButtonVariant::Subtle, ImVec2(halfW, 0.0f))) {
                    parallax::Notify(parallax::NotificationLevel::Info, "Inspector", "Opened runtime diagnostics stream.", 3.0f);
                }
                ImGui::SameLine(0.0f, 8.0f * dpi);
                if (parallax::Button("Purge Cache", "trash-2", parallax::ButtonVariant::Destructive, ImVec2(halfW, 0.0f))) {
                    parallax::Notify(parallax::NotificationLevel::Danger, "Cache Purged", "Cleared temporary icon and config buffers.", 3.5f);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Compact & Icon buttons row
                parallax::ButtonCompact("Compact Action", "check", parallax::ButtonVariant::Primary);
                ImGui::SameLine(0.0f, 6.0f * dpi);
                parallax::ButtonCompact("Secondary Pill", "sliders", parallax::ButtonVariant::Secondary);
                ImGui::SameLine(0.0f, 6.0f * dpi);
                parallax::IconButton("##QuickBellBtn", "bell", parallax::ButtonVariant::Secondary, 28.0f, "Send Test Ping");
                ImGui::SameLine(0.0f, 6.0f * dpi);
                parallax::IconButton("##QuickLockBtn", "lock", parallax::ButtonVariant::Ghost, 28.0f, "Open Key Modal");

                parallax::EndGroupbox();
                SpotlightCheck("Button Family & Variants");

                parallax::NextColumn();

                // Column 2: Sliders & Dual Range Sliders
                parallax::BeginGroupbox("Precision Sliders & Dual Range Sliders", "sliders");
                parallax::GroupboxDescription("Click value badge or Ctrl+Click track for inline numeric input. Drag handles for floating callout.");

                parallax::SliderFloat("Field of View", &fovValue, 60.0f, 130.0f, "%.1f deg");
                parallax::SliderFloat("Smoothing Factor (Compact)", &smoothingValue, 1.0f, 15.0f, "%.2fx", 0.0f, true);
                parallax::SliderFloatEx("Exposure Compensation (Ticks)", &exposureStop, -2.0f, 2.0f, {"%+.2f EV", 0.0f, false, false, false, 9, true});
                parallax::SliderInt("Priority Entity Cap", &maxTargetCount, 1, 16, "%d targets");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                parallax::RangeSliderFloat("Render Distance Window", &renderDistanceMin, &renderDistanceMax, 0.0f, 500.0f, "%.0f m");
                parallax::RangeSliderInt("Acceptable Ping Window", &pingRangeMin, &pingRangeMax, 0, 250, "%d ms");

                parallax::EndGroupbox();
                SpotlightCheck("Precision Sliders & Dual Range Sliders");

                // Second auto-sized card in Column 2: Collapsible Sections
                if (parallax::BeginCollapsibleGroupbox("Collapsible Acoustic Section", &audioGroupExpanded, "bell", 0.0f, false)) {
                    parallax::Row("Audio Alert Cues", &soundAlerts).Icon("bell");
                    static float alertGain = 78.0f;
                    parallax::SliderFloat("Master Alert Gain", &alertGain, 0.0f, 100.0f, "%.0f%%");
                    parallax::EndCollapsibleGroupbox();
                    SpotlightCheck("Collapsible Acoustic Section");
                }

                parallax::EndGrid();
            }
            // =================================================================
            // TAB 1: SELECTORS & INPUTS
            // (Dropdowns, Multi-Dropdowns, Inputs, Keybinds, Color Pickers)
            // =================================================================
            else if (activeMainTab == 1) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Single & Multi-Select Dropdowns + Color Pickers
                parallax::BeginGroupbox("Dropdowns & Multi-Select Menus", "layers");
                parallax::GroupboxDescription("Search-filtered popup menus with left accent bar, count badges, and custom checkboxes.");

                parallax::Dropdown("Target Bone Priority", hitboxItems, &selectedHitbox, hitboxIcons);
                ImGui::Spacing();
                parallax::Dropdown("Solver Interpolation", filterAlgorithmItems, &selectedAlgorithm, filterAlgorithmIcons);
                ImGui::Spacing();
                parallax::MultiDropdown("Active Telemetry Layers", visualElements, selectedElements, visualIcons);

                parallax::EndGroupbox();
                SpotlightCheck("Dropdowns & Multi-Select Menus");

                parallax::BeginGroupbox("Color Picker & Swatch Controls", "palette");
                parallax::GroupboxDescription("HSV saturation/value canvas, alpha checkerboard, HEX/RGBA fields, and right-click savable palette.");

                parallax::ColorPickerRow("Primary Bounding Color", &boxColor, true);
                parallax::ColorPickerRow("Trajectory Line Color", &snaplineColor, true);
                parallax::ColorPickerRow("Surface Rim Highlight", &glowColor, true);

                parallax::EndGroupbox();
                SpotlightCheck("Color Picker & Swatch Controls");

                parallax::NextColumn();

                // Column 2: Text Inputs & Hardware Keybind Controls
                parallax::BeginGroupbox("Text Inputs & Password Masking", "type");
                parallax::GroupboxDescription("Recessed control surfaces with animated focus border, clear button, and password visibility toggle.");

                parallax::InputText("Entity Filter Query", testSearchInput, "search", "Filter active entities...", -1.0f);
                ImGui::Spacing();
                parallax::InputText("Operator Identifier", testUsernameInput, "user", "Enter operator handle...", -1.0f);
                ImGui::Spacing();
                parallax::InputText("Secret API Token", testApiToken, "lock", "Enter secret key...", -1.0f, true);

                parallax::EndGroupbox();
                SpotlightCheck("Text Inputs & Password Masking");

                parallax::BeginGroupbox("Interactive Keybind Controls", "cpu");
                parallax::GroupboxDescription("Left-click to bind keyboard or mouse button (Esc clears). Right-click to select Hold / Toggle / Always On.");

                parallax::KeybindRow("Overlay Master Toggle", "Toggles 2D/3D bounding boxes", &boxKey);
                ImGui::Spacing();
                parallax::KeybindRow("Precision Aim Assist", "Active while key is held down", &aimKey);
                ImGui::Spacing();
                parallax::KeybindRow("Auto-Trigger Gate", "Continuous automatic activation", &triggerKey);
                ImGui::Spacing();
                parallax::KeybindRow("Surface Rim Shader", "Toggles outer contour pass", &glowKey);

                parallax::EndGroupbox();
                SpotlightCheck("Interactive Keybind Controls");

                parallax::EndGrid();
            }
            // =================================================================
            // TAB 2: THEMES & APPEARANCE
            // (AMOLED Black / Graphite Dark / Porcelain Light, Parallax & Accent Presets, Typography, Toasts)
            // =================================================================
            else if (activeMainTab == 2) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Theme Engine & Accent Presets
                parallax::BeginGroupbox("Theme Engine — AMOLED, Graphite & Porcelain", "sun");
                parallax::GroupboxDescription("Switch smoothly between AMOLED (Black), Graphite (Dark), and Porcelain (Light) with real-time token interpolation.");

                parallax::ThemeSwitcher("Active Interface Theme");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                parallax::AccentSelector("Restrained Accent Palette", true);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                parallax::PushFont(parallax::FontWeight::Medium);
                ImGui::TextColored(colors.TextSecondary, "Global Animation Speed Multiplier");
                parallax::PopFont();
                ImGui::Spacing();

                const auto speed = parallax::GetSpeedMultiplier();
                const float quarterW = (ImGui::GetContentRegionAvail().x - 18.0f * dpi) * 0.25f;
                if (parallax::Button("Fast 1.5x", nullptr, speed == parallax::SpeedMultiplier::Fast ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary, ImVec2(quarterW, 30.0f * dpi))) {
                    parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Fast);
                }
                ImGui::SameLine(0.0f, 6.0f * dpi);
                if (parallax::Button("Normal 1x", nullptr, speed == parallax::SpeedMultiplier::Normal ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary, ImVec2(quarterW, 30.0f * dpi))) {
                    parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Normal);
                }
                ImGui::SameLine(0.0f, 6.0f * dpi);
                if (parallax::Button("Relaxed", nullptr, speed == parallax::SpeedMultiplier::Relaxed ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary, ImVec2(quarterW, 30.0f * dpi))) {
                    parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Relaxed);
                }
                ImGui::SameLine(0.0f, 6.0f * dpi);
                if (parallax::Button("Instant", nullptr, speed == parallax::SpeedMultiplier::Off ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary, ImVec2(quarterW, 30.0f * dpi))) {
                    parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Off);
                }

                parallax::EndGroupbox();
                SpotlightCheck("Theme Engine — AMOLED, Graphite & Porcelain");

                parallax::NextColumn();

                // Column 2: Typography Scale & Toast Notifications
                parallax::BeginGroupbox("Typography Scale & Hierarchy", "type");
                parallax::GroupboxDescription("5-layer surface separation paired with DPI-scaled Inter font weights.");

                parallax::PushFont(parallax::FontWeight::Title);
                ImGui::TextColored(colors.TextPrimary, "Title — 22px Inter Bold");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Bold);
                ImGui::TextColored(colors.TextPrimary, "Section Heading — 18.5px Inter Bold");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::SemiBold);
                ImGui::TextColored(colors.TextPrimary, "Card Header — 16.5px Inter SemiBold");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Medium);
                ImGui::TextColored(colors.TextSecondary, "Control Label — 15px Inter Medium");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Regular);
                ImGui::TextColored(colors.TextSecondary, "Body Copy — 14.5px Inter Regular");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Caption);
                ImGui::TextColored(colors.TextMuted, "Caption & Metadata — 12.5px Inter Regular");
                parallax::PopFont();

                parallax::EndGroupbox();
                SpotlightCheck("Typography Scale & Hierarchy");

                parallax::BeginGroupbox("Toast Notification System", "bell");
                parallax::GroupboxDescription("Trigger theme-aware toast notifications with semantic status accents.");

                const float halfBtnW = (ImGui::GetContentRegionAvail().x - 8.0f * dpi) * 0.5f;
                if (parallax::Button("Success Toast", "check", parallax::ButtonVariant::Primary, ImVec2(halfBtnW, 34.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Success, "Verification Passed", "All design tokens and shaders validated.", 3.5f);
                }
                ImGui::SameLine(0.0f, 8.0f * dpi);
                if (parallax::Button("Info Toast", "bell", parallax::ButtonVariant::Secondary, ImVec2(halfBtnW, 34.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Info, "Theme Synchronized", "Active palette applied across all surfaces.", 3.5f);
                }
                ImGui::Spacing();
                if (parallax::Button("Warning Toast", "alert-triangle", parallax::ButtonVariant::Secondary, ImVec2(halfBtnW, 34.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Warning, "Latency Threshold", "Frame delta exceeded 14.2ms target.", 3.5f);
                }
                ImGui::SameLine(0.0f, 8.0f * dpi);
                if (parallax::Button("Danger Toast", "alert-circle", parallax::ButtonVariant::Destructive, ImVec2(halfBtnW, 34.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Danger, "Signature Mismatch", "Untrusted module blocked by policy.", 4.0f);
                }

                parallax::EndGroupbox();
                SpotlightCheck("Toast Notification System");

                parallax::EndGrid();
            }
            // =================================================================
            // TAB 3: LAYOUT EXAMPLES
            // (SubTabs, Layout Presets, Content-Fitting Cards, 1/2/3-Column Grids)
            // =================================================================
            else if (activeMainTab == 3) {
                parallax::BeginSubTabs(parallax::SubTabPlacement::TopHorizontal);
                if (parallax::SubTab("Navigation & Presets", activeLayoutSubTab == 0)) activeLayoutSubTab = 0;
                if (parallax::SubTab("Auto-Sizing Card Grid", activeLayoutSubTab == 1)) activeLayoutSubTab = 1;
                if (parallax::SubTab("Three-Column Telemetry", activeLayoutSubTab == 2)) activeLayoutSubTab = 2;
                parallax::EndSubTabs();

                if (activeLayoutSubTab == 0) {
                    const auto curPreset = parallax::GetLayoutPreset();
                    parallax::BeginGrid(2, 14.0f);

                    parallax::BeginGroupbox("Navigation Presets & Placement", "layout");
                    parallax::GroupboxDescription("Switch navigation dock placement or apply a complete layout preset.");

                    if (parallax::Button("Modern Sidebar (Expanded 200px Left Dock)", "sidebar",
                        curPreset == parallax::LayoutPreset::ModernSidebar ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary,
                        ImVec2(-1.0f, 36.0f * dpi)))
                    {
                        parallax::SetLayoutPreset(parallax::LayoutPreset::ModernSidebar);
                    }
                    ImGui::Spacing();
                    if (parallax::Button("Compact Tactical (Collapsed 68px Icon Dock)", "crosshair",
                        curPreset == parallax::LayoutPreset::CompactTactical ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary,
                        ImVec2(-1.0f, 36.0f * dpi)))
                    {
                        parallax::SetLayoutPreset(parallax::LayoutPreset::CompactTactical);
                    }
                    ImGui::Spacing();
                    if (parallax::Button("Minimal Tabbed (Top Horizontal Bar)", "maximize-2",
                        curPreset == parallax::LayoutPreset::MinimalTabbed ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary,
                        ImVec2(-1.0f, 36.0f * dpi)))
                    {
                        parallax::SetLayoutPreset(parallax::LayoutPreset::MinimalTabbed);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    parallax::PushFont(parallax::FontWeight::Medium);
                    ImGui::TextColored(colors.TextSecondary, "Navigation Dock Side:");
                    parallax::PopFont();
                    ImGui::Spacing();
                    auto side = parallax::GetNavSide();
                    parallax::layout::HeaderWidgets::NavSideSelector(side);
                    if (side != parallax::GetNavSide()) {
                        parallax::SetNavSide(side);
                    }

                    parallax::EndGroupbox();
                    SpotlightCheck("Navigation Presets & Placement");

                    parallax::NextColumn();

                    parallax::BeginGroupbox("Fixed Menu Shell & Automatic Card Sizing", "maximize");
                    parallax::GroupboxDescription("The outer menu maintains a fixed 1137x699 footprint while cards size to their content.");
                    ImGui::BulletText("Fixed window size: 1137 x 699 logical pixels");
                    ImGui::BulletText("Cards with height = 0.0f fit their exact child controls");
                    ImGui::BulletText("Multiple cards stack cleanly in each column without stretching");
                    ImGui::BulletText("Sidebar smoothly interpolates between 68px and 200px");
                    parallax::EndGroupbox();
                    SpotlightCheck("Fixed Menu Shell & Automatic Card Sizing");

                    if (parallax::BeginCollapsibleGroupbox("Collapsible Trajectory Solver", &advancedGroupExpanded, "cpu", 0.0f, false)) {
                        static bool enablePrediction = true;
                        static float solverHz = 120.0f;
                        parallax::Row("Sub-frame Kinematics", &enablePrediction).Icon("zap");
                        parallax::SliderFloat("Solver Tick Rate", &solverHz, 30.0f, 240.0f, "%.0f Hz");
                        parallax::EndCollapsibleGroupbox();
                        SpotlightCheck("Collapsible Trajectory Solver");
                    }

                    parallax::EndGrid();
                } else if (activeLayoutSubTab == 1) {
                    parallax::BeginGrid(2, 14.0f);

                    parallax::BeginGroupbox("Compact Card A (2 Controls)", "layers");
                    parallax::GroupboxDescription("This card contains only two rows and automatically fits its height.");
                    parallax::Row("Quick Toggle Alpha", &compactToggleA).Icon("check");
                    parallax::Row("Quick Toggle Beta", &compactToggleB).Icon("shield");
                    parallax::EndGroupbox();
                    SpotlightCheck("Compact Card A (2 Controls)");

                    parallax::BeginGroupbox("Compact Card B (Stacked Below A)", "sliders");
                    parallax::GroupboxDescription("Stacked directly below Card A in Column 1 with balanced spacing.");
                    parallax::SliderFloat("Local Gain", &smoothingValue, 1.0f, 15.0f, "%.1f dB");
                    parallax::EndGroupbox();

                    parallax::NextColumn();

                    parallax::BeginGroupbox("Medium Card C (4 Controls)", "eye");
                    parallax::GroupboxDescription("Demonstrates independent card heights across adjacent columns.");
                    parallax::Row("Box Overlay", &boxEsp).Color(&boxColor);
                    parallax::Row("Snaplines", &snaplines).Color(&snaplineColor);
                    parallax::Row("Skeletons", &skeletons).Color(&skeletonColor);
                    parallax::Row("Rim Glow", &glowEffect).Color(&glowColor);
                    parallax::EndGroupbox();

                    parallax::EndGrid();
                } else {
                    parallax::BeginGrid(3, 12.0f);

                    parallax::BeginGroupbox("Frame Timing", "clock");
                    parallax::PushFont(parallax::FontWeight::Title);
                    ImGui::TextColored(colors.Accent, "%.2f ms", 1000.0f / std::max(1.0f, ImGui::GetIO().Framerate));
                    parallax::PopFont();
                    ImGui::TextColored(colors.TextMuted, "Render Frame Time (%.0f FPS)", ImGui::GetIO().Framerate);
                    parallax::EndGroupbox();

                    parallax::NextColumn();

                    parallax::BeginGroupbox("Theme & Contrast", "sun");
                    parallax::PushFont(parallax::FontWeight::Title);
                    ImGui::TextColored(colors.TextPrimary, "%s", parallax::IsLightTheme() ? "Porcelain" : "Graphite");
                    parallax::PopFont();
                    ImGui::TextColored(colors.TextMuted, "5-Layer Surface Hierarchy");
                    parallax::EndGroupbox();

                    parallax::NextColumn();

                    parallax::BeginGroupbox("Config Binding", "database");
                    parallax::PushFont(parallax::FontWeight::Title);
                    ImGui::TextColored(colors.Success, "Active");
                    parallax::PopFont();
                    ImGui::TextColored(colors.TextMuted, "Profile: %s.json", cfg.GetActiveProfile().c_str());
                    parallax::EndGroupbox();

                    parallax::EndGrid();
                }
            }
            // =================================================================
            // TAB 4: ICONS (2,077 SVG Icon Atlas)
            // =================================================================
            else if (activeMainTab == 4) {
                parallax::BeginGroupbox("On-Demand 2,077 SVG Icon Atlas", "image");
                parallax::GroupboxDescription("Icons are rasterized via LunaSVG at native display DPI and packed into a dynamic DirectX 11 texture atlas.");

                const char* showcaseIcons[] = {
                    "shield", "activity", "sliders", "palette", "zap", "bell", "cpu", "layers",
                    "user", "search", "check", "folder", "database", "terminal", "lock", "globe",
                    "camera", "wifi", "battery-charging", "cloud", "mail", "flag", "heart", "star",
                    "navigation", "crosshair", "target", "circle", "sun", "moon", "clock", "award"
                };

                int shown = 0;
                for (size_t i = 0; i < sizeof(showcaseIcons) / sizeof(showcaseIcons[0]); ++i) {
                    if (!searchQuery.empty()) {
                        std::string nameStr = showcaseIcons[i];
                        std::string qStr = searchQuery;
                        std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                        std::transform(qStr.begin(), qStr.end(), qStr.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                        if (nameStr.find(qStr) == std::string::npos) continue;
                    }

                    if (shown % 8 != 0) ImGui::SameLine(0.0f, 16.0f * dpi);
                    ImGui::BeginGroup();
                    const ImVec2 cellPos = ImGui::GetCursorScreenPos();
                    const ImVec2 cellSize(96.0f * dpi, 54.0f * dpi);
                    ImDrawList* cellDraw = ImGui::GetWindowDrawList();
                    cellDraw->AddRectFilled(cellPos, ImVec2(cellPos.x + cellSize.x, cellPos.y + cellSize.y), ImColor(colors.WidgetBg), 6.0f * dpi);
                    cellDraw->AddRect(cellPos, ImVec2(cellPos.x + cellSize.x, cellPos.y + cellSize.y), ImColor(colors.BorderSubtle), 6.0f * dpi);

                    const float iconSz = 20.0f * dpi;
                    const ImVec2 iconPos(cellPos.x + (cellSize.x - iconSz) * 0.5f, cellPos.y + 8.0f * dpi);
                    parallax::Context::Get().Icons().DrawIcon(cellDraw, showcaseIcons[i], iconPos, ImVec2(iconSz, iconSz), colors.Accent);

                    parallax::PushFont(parallax::FontWeight::Caption);
                    const ImVec2 ts = ImGui::CalcTextSize(showcaseIcons[i]);
                    const ImVec2 labelPos(cellPos.x + (cellSize.x - ts.x) * 0.5f, cellPos.y + 32.0f * dpi);
                    cellDraw->AddText(labelPos, ImColor(colors.TextSecondary), showcaseIcons[i]);
                    parallax::PopFont();

                    ImGui::Dummy(cellSize);
                    ImGui::EndGroup();
                    shown++;
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                parallax::Icon("missing_icon_graceful_test", ImVec2(20.0f * dpi, 20.0f * dpi), colors.Warning);
                ImGui::SameLine(0.0f, 10.0f * dpi);
                ImGui::TextColored(colors.TextMuted, "Graceful Fallback: Unrecognized icon names automatically render the procedural vector shield.");
                parallax::EndGroupbox();
                SpotlightCheck("On-Demand 2,077 SVG Icon Atlas");
            }
            // =================================================================
            // TAB 5: CONFIGURATIONS (JSON Config Engine)
            // =================================================================
            else if (activeMainTab == 5) {
                cfg.RenderConfigTab();
            }
            // =================================================================
            // TAB 6: KEY SYSTEM (Developer Licensing & Modal)
            // =================================================================
            else if (activeMainTab == 6) {
                auto& keySys = parallax::auth::KeySystem::Get();
                parallax::BeginGrid(2, 14.0f);

                parallax::BeginGroupbox("Developer License Status", "shield");
                parallax::GroupboxDescription("Asynchronous seat validation and machine-persisted license state.");

                const bool isAuth = keySys.IsAuthenticated();
                if (isAuth) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(colors.Success.x, colors.Success.y, colors.Success.z, 0.14f));
                    ImGui::PushStyleColor(ImGuiCol_Border, colors.Success);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding * dpi);
                    ImGui::BeginChild("##AuthSuccessBox", ImVec2(-1.0f, 74.0f * dpi), true, ImGuiWindowFlags_NoScrollbar);

                    parallax::PushFont(parallax::FontWeight::Bold);
                    ImGui::TextColored(colors.Success, "STATUS: ACTIVE // AUTHENTICATED");
                    parallax::PopFont();

                    ImGui::TextColored(colors.TextPrimary, "Licensee: Alex Mercer (Pro Seat)");
                    ImGui::TextColored(colors.TextMuted, "Key: %s", keySys.GetEnteredKey().empty() ? "PRLX-2026-VIP" : keySys.GetEnteredKey().c_str());

                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                    if (parallax::Button("Invalidate / Logout", "lock", parallax::ButtonVariant::Destructive, ImVec2(-1.0f, 36.0f * dpi))) {
                        keySys.Logout();
                        parallax::Notify(parallax::NotificationLevel::Warning, "Logged Out", "Developer license invalidated.", 3.5f);
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(colors.Danger.x, colors.Danger.y, colors.Danger.z, 0.14f));
                    ImGui::PushStyleColor(ImGuiCol_Border, colors.Danger);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics.FrameRounding * dpi);
                    ImGui::BeginChild("##AuthLockedBox", ImVec2(-1.0f, 64.0f * dpi), true, ImGuiWindowFlags_NoScrollbar);

                    parallax::PushFont(parallax::FontWeight::Bold);
                    ImGui::TextColored(colors.Danger, "STATUS: LOCKED // AUTH REQUIRED");
                    parallax::PopFont();
                    ImGui::TextColored(colors.TextMuted, "Enter valid developer credentials to unlock full suite.");

                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                    if (parallax::Button("Open Key Authorization Modal", "unlock", parallax::ButtonVariant::Primary, ImVec2(-1.0f, 36.0f * dpi))) {
                        showKeyModal = true;
                    }
                }

                parallax::EndGroupbox();
                SpotlightCheck("Developer License Status");

                parallax::NextColumn();

                parallax::BeginGroupbox("Authorization Simulator", "terminal");
                parallax::GroupboxDescription("Test non-blocking verification with built-in demo keys or open the modal dialog.");

                if (parallax::Button("Authorize with Demo VIP Key (PRLX-2026-VIP)", "check", parallax::ButtonVariant::Secondary, ImVec2(-1.0f, 36.0f * dpi))) {
                    keySys.StartVerification("PRLX-2026-VIP");
                    parallax::Notify(parallax::NotificationLevel::Info, "Verifying Key", "Validating PRLX-2026-VIP...", 2.5f);
                }
                ImGui::Spacing();
                if (parallax::Button("Authorize with Demo Pro Key (PRLX-DEMO-PRO)", "check", parallax::ButtonVariant::Secondary, ImVec2(-1.0f, 36.0f * dpi))) {
                    keySys.StartVerification("PRLX-DEMO-PRO");
                    parallax::Notify(parallax::NotificationLevel::Info, "Verifying Key", "Validating PRLX-DEMO-PRO...", 2.5f);
                }
                ImGui::Spacing();
                if (parallax::Button("Trigger Invalid Key Failure Test", "alert-triangle", parallax::ButtonVariant::Destructive, ImVec2(-1.0f, 36.0f * dpi))) {
                    keySys.StartVerification("INVALID-TEST-KEY-000");
                    parallax::Notify(parallax::NotificationLevel::Danger, "Verification", "Testing rejection behavior...", 3.0f);
                }
                ImGui::Spacing();
                if (parallax::Button("Launch Key Authorization Modal", "external-link", parallax::ButtonVariant::Ghost, ImVec2(-1.0f, 34.0f * dpi))) {
                    showKeyModal = true;
                }

                parallax::EndGroupbox();
                SpotlightCheck("Authorization Simulator");

                parallax::EndGrid();
            }

            ImGui::EndChild();
            parallax::EndWindow();
        }
    });

    // 5. Cleanup
    parallax::Shutdown();
    host.Shutdown();

    return 0;
}

int main(int, char**) {
    return RunDemo(GetModuleHandle(nullptr));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    return RunDemo(hInstance);
}
