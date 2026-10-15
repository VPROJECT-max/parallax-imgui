#include "parallax/parallax.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

int RunDemo(HINSTANCE hInstance) {
    // 1. Initialize standalone DirectX 11 Host window with Immersive Dark Mode
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1180, 742, L"Parallax UI DX11 - Modern C++20 UI/UX Framework")) {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 Host Window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2. Initialize Parallax UI Library (Loads Inter Fonts, Metrics & SVG Atlas)
    parallax::Init(host.GetDevice(), host.GetContext());

    // Initial welcome notification
    parallax::Notify(
        parallax::NotificationLevel::Success,
        "Parallax v3.0 Active",
        "Inter typography loaded, custom widgets & layout presets initialized.",
        5.0f
    );

    // 3. Demo application state
    bool menuOpen = true;
    int activeMainTab = 0;
    std::string searchQuery = "";

    // Text inputs state
    std::string testSearchInput = "";
    std::string testUsernameInput = "alex.mercer";
    std::string testApiToken = "sk-live-94a82f0c11d";

    // Recent activity log for live interaction visual feedback
    std::deque<std::string> activityLog;
    auto logEvent = [&](const std::string& msg) {
        activityLog.push_front(msg);
        if (activityLog.size() > 8) activityLog.pop_back();
    };
    logEvent("System initialized with Inter font hierarchy and DirectX 11.");

    // Phase 2 & 3: Chained Rows & Toggle State
    bool boxEsp = true;
    bool snaplines = false;
    bool skeletons = true;
    bool glowEffect = true;
    bool soundAlerts = true;
    bool disabledDemo = false;

    // Keybind State
    parallax::Keybind boxKey{ 'B', parallax::KeybindMode::Toggle, false };
    parallax::Keybind aimKey{ VK_XBUTTON1, parallax::KeybindMode::Hold, false };
    parallax::Keybind triggerKey{ VK_SPACE, parallax::KeybindMode::AlwaysOn, true };
    parallax::Keybind glowKey{ 'G', parallax::KeybindMode::Toggle, false };

    // Color Picker State
    ImVec4 boxColor{ 0.388f, 0.400f, 0.945f, 1.0f };   // Electric Indigo #6366F1
    ImVec4 snaplineColor{ 0.937f, 0.267f, 0.267f, 0.90f }; // Crimson #EF4444
    ImVec4 skeletonColor{ 0.95f, 0.95f, 0.95f, 1.0f };
    ImVec4 glowColor{ 0.133f, 0.773f, 0.369f, 0.90f }; // Emerald #22C55E

    // Dropdown State
    std::vector<std::string> hitboxItems = { "Head Target", "Neck / Spine", "Upper Torso", "Lower Pelvis", "Closest Bone" };
    std::vector<std::string> hitboxIcons = { "target", "crosshair", "shield", "circle", "zap" };
    int selectedHitbox = 0;

    std::vector<std::string> visualElements = { "Health Gauge", "Shield Level", "Distance Tag", "Active Weapon", "Player Identity", "Offscreen Radar" };
    std::vector<std::string> visualIcons = { "heart", "shield", "map-pin", "zap", "user", "navigation" };
    std::vector<bool> selectedElements = { true, true, true, false, true, false };

    // Sliders & Range Sliders
    float fovValue = 90.0f;
    float smoothingValue = 4.5f;
    int maxTargetCount = 6;
    float renderDistanceMin = 15.0f;
    float renderDistanceMax = 320.0f;
    int pingRangeMin = 20;
    int pingRangeMax = 120;

    // Collapsible Groupbox States
    bool visualGroupExpanded = true;
    bool audioGroupExpanded = true;
    bool advancedGroupExpanded = false;

    // 4. Main message and render loop
    host.RunMainLoop([&]() {
        // Update live keybind states
        parallax::UpdateKeybindState(boxKey);
        parallax::UpdateKeybindState(aimKey);
        parallax::UpdateKeybindState(triggerKey);
        parallax::UpdateKeybindState(glowKey);

        // Always render floating watermark status pill
        parallax::Watermark("PARALLAX v3.0", "Alex Mercer (Pro)", true, true);

        // Always render toast notifications
        parallax::RenderNotifications();

        // Draw guidance text when menu is closed
        if (!menuOpen) {
            ImGui::SetNextWindowPos(ImVec2(30.0f, 60.0f));
            ImGui::Begin("##ParallaxBackgroundHint", nullptr,
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
            parallax::PushFont(parallax::FontWeight::SemiBold);
            ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 0.9f), "Press [Right Shift] or [Insert] to open Parallax");
            parallax::PopFont();
            ImGui::End();
        }

        // Render Parallax Window
        if (parallax::BeginWindow("Parallax Control System", &menuOpen)) {
            const float dpi = parallax::GetDpiScale();
            const auto& metrics = parallax::Metrics();
            const auto& colors = parallax::Colors();
            const ImVec2 winPos = ImGui::GetWindowPos();
            const ImVec2 winSize = ImGui::GetWindowSize();
            const auto navSide = parallax::GetNavSide();
            const auto curPreset = parallax::GetLayoutPreset();

            // -----------------------------------------------------------------
            // Navigation Bar
            // -----------------------------------------------------------------
            parallax::BeginNavBar();
            if (parallax::NavTab("sliders", "Widgets", activeMainTab == 0, "CUSTOM")) activeMainTab = 0;
            if (parallax::NavTab("layers", "Selectors", activeMainTab == 1)) activeMainTab = 1;
            if (parallax::NavTab("layout", "Presets & Sizing", activeMainTab == 2)) activeMainTab = 2;
            if (parallax::NavTab("type", "Typography", activeMainTab == 3)) activeMainTab = 3;
            if (parallax::NavTab("folder", "Collapsible", activeMainTab == 4)) activeMainTab = 4;
            if (parallax::NavTab("image", "SVG Atlas", activeMainTab == 5)) activeMainTab = 5;

            // User Profile Card at bottom of vertical sidebars
            if (navSide == parallax::NavSide::Left || navSide == parallax::NavSide::Right) {
                const float curNavW = parallax::GetNavWidth() * dpi;
                const float profileCardW = curNavW - 16.0f * dpi;
                const float profileY = winPos.y + winSize.y - (metrics.ProfileCardHeight + 14.0f) * dpi;
                float profileX = (navSide == parallax::NavSide::Right) ? (winPos.x + winSize.x - curNavW + 8.0f * dpi) : (winPos.x + 8.0f * dpi);
                ImGui::SetCursorScreenPos(ImVec2(profileX, profileY));
                parallax::UserProfileCard(parallax::Context::Get().Profile, ImVec2(profileCardW, metrics.ProfileCardHeight * dpi));
            }
            parallax::EndNavBar();

            // -----------------------------------------------------------------
            // Content Region Bounds Calculations (Dynamic Internal Allocation)
            // -----------------------------------------------------------------
            const float dynamicNavW = parallax::GetNavWidth() * dpi;
            ImVec2 contentPos;
            ImVec2 contentSize;

            switch (navSide) {
                case parallax::NavSide::Left:
                    contentPos = ImVec2(winPos.x + dynamicNavW + 16.0f * dpi, winPos.y + 16.0f * dpi);
                    contentSize = ImVec2(winSize.x - dynamicNavW - 32.0f * dpi, winSize.y - 32.0f * dpi);
                    break;
                case parallax::NavSide::Right:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 16.0f * dpi);
                    contentSize = ImVec2(winSize.x - dynamicNavW - 32.0f * dpi, winSize.y - 32.0f * dpi);
                    break;
                case parallax::NavSide::Top:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + metrics.NavHeight * dpi + 16.0f * dpi);
                    contentSize = ImVec2(winSize.x - 32.0f * dpi, winSize.y - metrics.NavHeight * dpi - 32.0f * dpi);
                    break;
                case parallax::NavSide::Bottom:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 16.0f * dpi);
                    contentSize = ImVec2(winSize.x - 32.0f * dpi, winSize.y - metrics.NavHeight * dpi - 32.0f * dpi);
                    break;
            }

            ImGui::SetCursorScreenPos(contentPos);
            ImGui::BeginChild("##ParallaxMainContentRegion", contentSize, false, ImGuiWindowFlags_None);

            // Smooth scrolling accumulation and spring interpolation
            static float s_targetScrollY = 0.0f;
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) {
                    s_targetScrollY -= wheel * 48.0f * dpi;
                }
            }
            const float maxScroll = ImGui::GetScrollMaxY();
            s_targetScrollY = std::clamp(s_targetScrollY, 0.0f, maxScroll);
            float smoothScroll = parallax::SpringFloat(ImGui::GetID("##MainSmoothScroll"), s_targetScrollY, {260.0f, 26.0f, 1.0f});
            if (std::abs(smoothScroll - s_targetScrollY) < 0.2f) smoothScroll = s_targetScrollY;
            ImGui::SetScrollY(smoothScroll);

            // -----------------------------------------------------------------
            // Top Bar: Search Bar & Preset Controls
            // -----------------------------------------------------------------
            ImGui::BeginGroup();
            parallax::SearchBar(searchQuery, "Search modules, options, settings...", 260.0f * dpi);
            ImGui::SameLine(0.0f, 16.0f * dpi);

            parallax::PushFont(parallax::FontWeight::Caption);
            ImGui::TextColored(colors.TextMuted, "Preset: %s", parallax::GetLayoutPresetName(curPreset));
            parallax::PopFont();

            ImGui::SameLine(0.0f, 20.0f * dpi);
            parallax::PushFont(parallax::FontWeight::Caption);
            ImGui::TextColored(colors.TextMuted, "Dimensions: %.0fx%.0f", winSize.x, winSize.y);
            parallax::PopFont();
            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Spacing();

            // -----------------------------------------------------------------
            // Tab 0: Custom Widgets (Buttons, Inputs, Chained Rows)
            // -----------------------------------------------------------------
            if (activeMainTab == 0) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Custom Buttons & Text Inputs
                parallax::BeginGroupbox("Custom Button Family & Text Inputs", "zap");
                ImGui::TextColored(colors.TextMuted, "Tactile button variants with spring hover/active recoil:");
                ImGui::Spacing();

                // Custom Buttons
                if (parallax::Button("Deploy Security Module", "shield", parallax::ButtonVariant::Primary, ImVec2(-1.0f, 38.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Success, "Security Module Deployed", "All kernel shields and memory watchdogs active.", 4.0f);
                    logEvent("Button clicked: Deploy Security Module");
                }

                ImGui::Spacing();

                if (parallax::Button("Synchronize Cloud Settings", "refresh-cw", parallax::ButtonVariant::Secondary, ImVec2(-1.0f, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Info, "Profile Synchronized", "Downloaded remote configuration profile.", 3.5f);
                    logEvent("Button clicked: Synchronize Cloud Settings");
                }

                ImGui::Spacing();

                // Danger & Ghost buttons in one row
                const float halfW = (ImGui::GetContentRegionAvail().x - 10.0f * dpi) * 0.5f;
                if (parallax::Button("Purge Cache", "trash-2", parallax::ButtonVariant::Danger, ImVec2(halfW, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Danger, "Cache Purged", "Temporary memory buffers and icons cleared.", 4.0f);
                    logEvent("Button clicked: Purge Cache");
                }
                ImGui::SameLine(0.0f, 10.0f * dpi);
                if (parallax::Button("Open Docs", "external-link", parallax::ButtonVariant::Ghost, ImVec2(halfW, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Warning, "Documentation", "Opened local HTML documentation in docs/.", 3.0f);
                    logEvent("Button clicked: Open Docs");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Custom Styled Text Inputs:");
                ImGui::Spacing();

                // Input with search icon
                parallax::InputText("Query Filter", testSearchInput, "search", "Filter target entities...", -1.0f);

                ImGui::Spacing();

                // Input with user icon
                parallax::InputText("Operator Handle", testUsernameInput, "user", "Enter username...", -1.0f);

                ImGui::Spacing();

                // Password masked input with lock icon
                parallax::InputText("Authorization Key", testApiToken, "lock", "Enter API key...", -1.0f, true);

                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Declarative Chained Rows
                parallax::BeginGroupbox("Declarative Chained Settings", "sliders");
                ImGui::TextColored(colors.TextMuted, "Custom-designed rows with icons, keybinds, and color swatches:");
                ImGui::Spacing();

                // 1. Pill Switch with Icon, Description, Tooltip, Keybind, Color Picker, and OnChange
                parallax::Row("Bounding Box Overlay", &boxEsp)
                    .Icon("eye")
                    .Description("Draw crisp dynamic 2D/3D boxes around targets")
                    .Tooltip("Supports customizable color swatch and hotkey trigger")
                    .Keybind(&boxKey)
                    .Color(&boxColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Bounding Box: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 2. Checkbox with Icon, Keybind, Color, Description
                parallax::Row("Snaplines", &snaplines)
                    .Style(parallax::ToggleStyle::Checkbox)
                    .Icon("navigation")
                    .Description("Draw directional trajectory line to target")
                    .Color(&snaplineColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Snaplines: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 3. Skeletons with Keybind and Color
                parallax::Row("Skeleton Joint Bones", &skeletons)
                    .Icon("activity")
                    .Description("Overlay anatomical skeletal joint structures")
                    .Color(&skeletonColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Skeletons: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 4. Glow Effect with Keybind & Color
                parallax::Row("Outer Surface Glow", &glowEffect)
                    .Icon("zap")
                    .Description("Harmonic edge outline shader effect")
                    .Keybind(&glowKey)
                    .Color(&glowColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Outer Glow: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 5. Disabled State Row Showcase
                parallax::Row("Hardware Depth Pass (Locked)", &disabledDemo)
                    .Icon("lock")
                    .Description("Requires DirectX 12 or Vulkan compute backend")
                    .Disabled(true);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Recent Event Stream:");
                ImGui::Spacing();

                ImGui::BeginChild("##DemoActivityLog", ImVec2(0.0f, 100.0f * dpi), true, ImGuiWindowFlags_NoScrollbar);
                for (const auto& logMsg : activityLog) {
                    parallax::PushFont(parallax::FontWeight::Caption);
                    ImGui::TextColored(colors.TextMuted, "• %s", logMsg.c_str());
                    parallax::PopFont();
                }
                ImGui::EndChild();

                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 1: Selectors & Sliders
            // -----------------------------------------------------------------
            else if (activeMainTab == 1) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Smooth Spring Sliders & Range Sliders
                parallax::BeginGroupbox("Tactile Sliders & Range Sliders", "sliders");
                ImGui::TextColored(colors.TextMuted, "Custom tracks, spring thumb interpolation & floating tooltips:");
                ImGui::Spacing();

                // Float Slider
                if (parallax::SliderFloat("Field of View", &fovValue, 60.0f, 130.0f, "%.1f deg")) {
                    logEvent("Field of View updated");
                }

                // 2-Line Float Slider
                if (parallax::SliderFloat("Aim Smoothing", &smoothingValue, 1.0f, 15.0f, "%.2f factor", 0.0f, true)) {
                    logEvent("Aim Smoothing updated");
                }

                // Integer Slider
                if (parallax::SliderInt("Target Priority Limit", &maxTargetCount, 1, 16, "%d entities")) {
                    logEvent("Target Priority Limit updated");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Dual-Thumb Range Sliders:");
                ImGui::Spacing();

                // Dual-Thumb Range Slider (Float)
                parallax::RangeSliderFloat("Render Distance Range", &renderDistanceMin, &renderDistanceMax, 0.0f, 500.0f, "%.0f m");

                // Dual-Thumb Range Slider (Int)
                parallax::RangeSliderInt("Ping Quality Bounds", &pingRangeMin, &pingRangeMax, 0, 250, "%d ms");

                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Single & Multi-Select Dropdowns
                parallax::BeginGroupbox("Searchable Dropdown Selectors", "layers");
                ImGui::TextColored(colors.TextMuted, "Custom popups, search filtering, icons, and selection badges:");
                ImGui::Spacing();

                // Single Select Dropdown
                if (parallax::Dropdown("Target Hitbox", hitboxItems, &selectedHitbox, hitboxIcons)) {
                    logEvent(std::string("Hitbox selected: ") + hitboxItems[selectedHitbox]);
                }

                ImGui::Spacing();
                ImGui::Spacing();

                // Searchable Multi-Select Dropdown
                if (parallax::MultiDropdown("Active ESP Indicators", visualElements, selectedElements, visualIcons)) {
                    int count = 0;
                    for (bool b : selectedElements) if (b) count++;
                    logEvent(std::string("ESP Indicators updated: ") + std::to_string(count) + " enabled");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Precision Color Picker Swatch:");
                ImGui::Spacing();
                ImGui::Text("Interface Accent Color:");
                ImGui::SameLine(180.0f * dpi);
                static ImVec4 customAccent{ 0.388f, 0.400f, 0.945f, 1.0f };
                parallax::ColorPicker("##CustomAccentColor", &customAccent, true);

                ImGui::SameLine(220.0f * dpi);
                parallax::PushFont(parallax::FontWeight::Caption);
                ImGui::TextColored(customAccent, "HEX: #%02X%02X%02X",
                    (int)(customAccent.x * 255.0f),
                    (int)(customAccent.y * 255.0f),
                    (int)(customAccent.z * 255.0f));
                parallax::PopFont();

                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 2: Layout Presets & Resizing Engine
            // -----------------------------------------------------------------
            else if (activeMainTab == 2) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Live Layout Preset Switcher
                parallax::BeginGroupbox("Game-Category Layout Presets", "layout");
                ImGui::TextWrapped("Switch between three production-grade layout presets at runtime:");
                ImGui::Spacing();

                // 1. Modern Sidebar
                if (parallax::Button("Modern Sidebar (Spacious 200px Left Bar)", "sidebar",
                    curPreset == parallax::LayoutPreset::ModernSidebar ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary,
                    ImVec2(-1.0f, 38.0f * dpi)))
                {
                    parallax::SetLayoutPreset(parallax::LayoutPreset::ModernSidebar);
                    parallax::Notify(parallax::NotificationLevel::Info, "Preset Changed", "Switched to Modern Sidebar layout.", 3.0f);
                }

                ImGui::Spacing();

                // 2. Compact Tactical
                if (parallax::Button("Compact Tactical (Dense 68px Icon Dock)", "crosshair",
                    curPreset == parallax::LayoutPreset::CompactTactical ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary,
                    ImVec2(-1.0f, 38.0f * dpi)))
                {
                    parallax::SetLayoutPreset(parallax::LayoutPreset::CompactTactical);
                    parallax::Notify(parallax::NotificationLevel::Info, "Preset Changed", "Switched to Compact Tactical layout.", 3.0f);
                }

                ImGui::Spacing();

                // 3. Minimal Tabbed
                if (parallax::Button("Minimal Tabbed (Clean Top Horizontal Bar)", "maximize-2",
                    curPreset == parallax::LayoutPreset::MinimalTabbed ? parallax::ButtonVariant::Primary : parallax::ButtonVariant::Secondary,
                    ImVec2(-1.0f, 38.0f * dpi)))
                {
                    parallax::SetLayoutPreset(parallax::LayoutPreset::MinimalTabbed);
                    parallax::Notify(parallax::NotificationLevel::Info, "Preset Changed", "Switched to Minimal Tabbed layout.", 3.0f);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Strict Fixed Dimensions — 1137 x 699:");
                ImGui::BulletText("Fixed logical dimensions: Width: 1137px, Height: 699px");
                ImGui::BulletText("Manual resizing and resize handles permanently disabled");
                ImGui::BulletText("Menu repositioning via window drag fully preserved");
                ImGui::BulletText("All content columns reflow deterministically");

                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Sidebar Control & Motion Multiplier
                parallax::BeginGroupbox("Sidebar Expansion & Motion Engine", "cpu");
                ImGui::TextColored(colors.TextSecondary, "Sidebar State Control:");
                ImGui::Spacing();

                bool isExpanded = parallax::IsNavExpanded();
                if (parallax::Button(isExpanded ? "Collapse Sidebar (68px)" : "Expand Sidebar (200px)",
                    isExpanded ? "chevron-left" : "chevron-right",
                    parallax::ButtonVariant::Primary,
                    ImVec2(-1.0f, 32.0f * dpi)))
                {
                    parallax::ToggleNavExpanded();
                }
                parallax::PushFont(parallax::FontWeight::Caption);
                ImGui::TextColored(colors.TextMuted, "Current Animated Width: %.1f px (Icons Centered when < 120px)", parallax::GetNavWidth());
                parallax::PopFont();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Global Animation Speed Multiplier:");
                auto speed = parallax::GetSpeedMultiplier();
                if (ImGui::RadioButton("Fast (1.5x)", speed == parallax::SpeedMultiplier::Fast)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Fast);
                ImGui::SameLine();
                if (ImGui::RadioButton("Normal (1.0x)", speed == parallax::SpeedMultiplier::Normal)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Normal);
                ImGui::SameLine();
                if (ImGui::RadioButton("Relaxed (0.7x)", speed == parallax::SpeedMultiplier::Relaxed)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Relaxed);
                ImGui::SameLine();
                if (ImGui::RadioButton("Off", speed == parallax::SpeedMultiplier::Off)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Off);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Harmonic Spring Motion Tester (k=100, c=20):");
                static float testSliderTarget = 65.0f;
                parallax::SliderFloat("Target Spring Value", &testSliderTarget, 0.0f, 100.0f, "%.1f");

                float currentSpringVal = parallax::SpringFloat(ImGui::GetID("##DemoSpringVal"), testSliderTarget, {100.0f, 20.0f, 1.0f});
                ImGui::ProgressBar(currentSpringVal / 100.0f, ImVec2(-1.0f, 18.0f * dpi), "");
                parallax::PushFont(parallax::FontWeight::Caption);
                ImGui::TextColored(colors.TextMuted, "Physically Settled Position: %.2f", currentSpringVal);
                parallax::PopFont();

                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 3: Typography Hierarchy & Toast Testbench
            // -----------------------------------------------------------------
            else if (activeMainTab == 3) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Typography System Hierarchy
                parallax::BeginGroupbox("Inter Font Weight Hierarchy", "type");
                ImGui::TextColored(colors.TextMuted, "Licensed Inter TrueType font loaded at native display DPI:");
                ImGui::Spacing();

                parallax::PushFont(parallax::FontWeight::Title);
                ImGui::TextColored(colors.TextPrimary, "Title (22px Inter Bold)");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Bold);
                ImGui::TextColored(colors.TextPrimary, "Section Heading (18.5px Inter Bold)");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::SemiBold);
                ImGui::TextColored(colors.TextPrimary, "Card Header (16.5px Inter SemiBold)");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Medium);
                ImGui::TextColored(colors.TextSecondary, "Control Label / Button (15px Inter Medium)");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Regular);
                ImGui::TextColored(colors.TextSecondary, "Body & Form Text (14.5px Inter Regular)");
                parallax::PopFont();

                parallax::PushFont(parallax::FontWeight::Caption);
                ImGui::TextColored(colors.TextMuted, "Description & Metadata (12.5px Inter Caption)");
                parallax::PopFont();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextWrapped("All typography is anti-aliased with high-precision subpixel rasterization and scales crisply with display DPI.");
                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Toast Notification Testbench
                parallax::BeginGroupbox("Toast Notification Testbench", "bell");
                ImGui::TextColored(colors.TextMuted, "Trigger spring-animated toast notifications with progress bars:");
                ImGui::Spacing();

                if (parallax::Button("Trigger Success Notification", "check", parallax::ButtonVariant::Primary, ImVec2(-1.0f, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Success, "Operation Completed", "All memory buffers validated and written.", 4.0f);
                }

                ImGui::Spacing();

                if (parallax::Button("Trigger Info Notification", "bell", parallax::ButtonVariant::Secondary, ImVec2(-1.0f, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Info, "New Update Available", "Parallax v3.1 update ready for installation.", 4.5f);
                }

                ImGui::Spacing();

                if (parallax::Button("Trigger Warning Notification", "alert-triangle", parallax::ButtonVariant::Secondary, ImVec2(-1.0f, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Warning, "High GPU Workload", "Frame render latency exceeded 16.6ms threshold.", 4.0f);
                }

                ImGui::Spacing();

                if (parallax::Button("Trigger Danger Notification", "alert-circle", parallax::ButtonVariant::Danger, ImVec2(-1.0f, 36.0f * dpi))) {
                    parallax::Notify(parallax::NotificationLevel::Danger, "Access Denied", "Kernel handle failed validation check.", 5.0f);
                }

                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 4: Collapsible Groupboxes
            // -----------------------------------------------------------------
            else if (activeMainTab == 4) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Centered Header & Divider Groupbox
                if (parallax::BeginCollapsibleGroupbox("Visuals & Overlays", &visualGroupExpanded, "eye", 0.0f, true)) {
                    ImGui::TextColored(colors.TextMuted, "Smooth spring-animated height expansion with child clipping.");
                    ImGui::Spacing();

                    parallax::Row("Bounding Box", &boxEsp).Color(&boxColor);
                    parallax::Row("Skeletal Bones", &skeletons).Color(&skeletonColor);
                    parallax::Row("Outer Surface Glow", &glowEffect).Color(&glowColor);

                    parallax::EndCollapsibleGroupbox();
                }

                ImGui::Spacing();

                // Collapsible Audio Groupbox
                if (parallax::BeginCollapsibleGroupbox("Acoustic & Sound Alerts", &audioGroupExpanded, "bell", 0.0f, true)) {
                    ImGui::TextColored(colors.TextMuted, "Audio feedback cues and hit confirm sounds.");
                    ImGui::Spacing();

                    parallax::Row("Hit Confirmation Beep", &soundAlerts).Style(parallax::ToggleStyle::PillSwitch);
                    static float soundVolume = 80.0f;
                    parallax::SliderFloat("Feedback Volume", &soundVolume, 0.0f, 100.0f, "%.0f%%");

                    parallax::EndCollapsibleGroupbox();
                }

                parallax::NextColumn();

                // Column 2: Left-Aligned Header with Icon
                if (parallax::BeginCollapsibleGroupbox("Predictive Trajectory Engine", &advancedGroupExpanded, "cpu", 0.0f, false)) {
                    ImGui::TextColored(colors.TextMuted, "High-precision calculations and predictive trajectory solver.");
                    ImGui::Spacing();

                    static bool predictPhysics = true;
                    static float predictionTicks = 12.0f;
                    parallax::Row("Predictive Trajectory", &predictPhysics).Icon("zap");
                    parallax::SliderFloat("Prediction Ticks", &predictionTicks, 1.0f, 32.0f, "%.0f ticks");

                    parallax::EndCollapsibleGroupbox();
                }

                parallax::BeginGroupbox("Responsive Layout Guidelines", "layers");
                ImGui::TextWrapped("The responsive grid automatically partitions available content space into equal columns with synchronized spacing.");
                ImGui::Spacing();
                ImGui::BulletText("Single Column (1): Full-width detailed forms");
                ImGui::BulletText("Two Columns (2): Optimal settings & property inspection");
                ImGui::BulletText("Three Columns (3): High-density dashboards & metrics");
                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 5: SVG Icon Atlas (2,077 Icons)
            // -----------------------------------------------------------------
            else if (activeMainTab == 5) {
                parallax::BeginGroupbox("On-Demand 2,077 SVG Icon Showcase (Dynamic Atlas)", "image");
                ImGui::TextWrapped("Icons are rasterized on first use at current display DPI and packed into a single dynamic DirectX 11 Texture Atlas:");
                ImGui::Spacing();

                const char* showcaseIcons[] = {
                    "shield", "activity", "sliders", "palette", "zap", "bell", "cpu", "layers",
                    "user", "search", "check", "folder", "database", "terminal", "lock", "globe",
                    "camera", "wifi", "battery-charging", "cloud", "mail", "flag", "heart", "star",
                    "navigation", "crosshair", "target", "circle", "move", "maximize", "clock", "award"
                };

                for (size_t i = 0; i < sizeof(showcaseIcons) / sizeof(showcaseIcons[0]); ++i) {
                    if (i % 8 != 0) ImGui::SameLine(0.0f, 18.0f * dpi);
                    ImGui::BeginGroup();
                    parallax::Icon(showcaseIcons[i], ImVec2(24.0f * dpi, 24.0f * dpi), colors.Accent);
                    parallax::PushFont(parallax::FontWeight::Caption);
                    ImGui::TextColored(colors.TextMuted, "%s", showcaseIcons[i]);
                    parallax::PopFont();
                    ImGui::EndGroup();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextColored(colors.TextSecondary, "Graceful Fallback Protection:");
                parallax::Icon("missing_icon_graceful_test", ImVec2(24.0f * dpi, 24.0f * dpi), colors.Warning);
                ImGui::SameLine(0.0f, 10.0f * dpi);
                ImGui::TextColored(colors.TextMuted, "Unrecognized icon names fallback cleanly to the procedural shield without crashing or throwing.");
                parallax::EndGroupbox();
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
