#include "parallax/parallax.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>

int RunDemo(HINSTANCE hInstance) {
    // 1. Initialize standalone DirectX 11 Host window
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1280, 720, L"Parallax UI DX11 Demo - Phase 2 Showcase")) {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 Host Window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2. Initialize Parallax UI Library
    parallax::Init(host.GetDevice(), host.GetContext());

    // 3. Demo application state
    bool menuOpen = true;
    int activeMainTab = 0;
    std::string searchQuery = "";
    std::string typingTestInput = "Type capital letters with Right Shift here...";

    // Recent activity log for live interaction visual feedback
    std::deque<std::string> activityLog;
    auto logEvent = [&](const std::string& msg) {
        activityLog.push_front(msg);
        if (activityLog.size() > 8) activityLog.pop_back();
    };
    logEvent("Parallax UI initialized successfully (DirectX 11 + ImGui).");

    // Phase 2: Chained Rows & Toggle State
    bool boxEsp = true;
    bool snaplines = false;
    bool skeletons = true;
    bool glowEffect = true;
    bool disabledDemo = false;
    bool soundAlerts = true;

    // Phase 2: Keybind State
    parallax::Keybind boxKey{ 'B', parallax::KeybindMode::Toggle, false };
    parallax::Keybind aimKey{ VK_XBUTTON1, parallax::KeybindMode::Hold, false };
    parallax::Keybind triggerKey{ VK_SPACE, parallax::KeybindMode::AlwaysOn, true };
    parallax::Keybind glowKey{ 'G', parallax::KeybindMode::Toggle, false };

    // Phase 2: Color Picker State
    ImVec4 boxColor{ 0.38f, 0.58f, 0.98f, 1.0f };
    ImVec4 snaplineColor{ 0.95f, 0.40f, 0.40f, 0.85f };
    ImVec4 skeletonColor{ 0.95f, 0.95f, 0.95f, 1.0f };
    ImVec4 glowColor{ 0.16f, 0.78f, 0.52f, 0.90f };

    // Phase 2: Dropdown & Multi-Dropdown State
    std::vector<std::string> hitboxItems = { "Head Target", "Neck / Spine", "Upper Torso", "Lower Pelvis", "Closest Bone" };
    std::vector<std::string> hitboxIcons = { "target", "crosshair", "shield", "circle", "zap" };
    int selectedHitbox = 0;

    std::vector<std::string> visualElements = { "Health Gauge", "Shield Level", "Distance Tag", "Active Weapon", "Player Identity", "Offscreen Radar" };
    std::vector<std::string> visualIcons = { "heart", "shield", "map-pin", "zap", "user", "navigation" };
    std::vector<bool> selectedElements = { true, true, true, false, true, false };

    // Phase 2: Sliders & Range Sliders
    float fovValue = 90.0f;
    float smoothingValue = 4.5f;
    int maxTargetCount = 6;
    float renderDistanceMin = 10.0f;
    float renderDistanceMax = 320.0f;
    int pingRangeMin = 15;
    int pingRangeMax = 110;

    // Phase 2: Collapsible Groupbox States
    bool visualGroupExpanded = true;
    bool audioGroupExpanded = true;
    bool advancedGroupExpanded = false;

    // 4. Run main message and render loop
    host.RunMainLoop([&]() {
        // Update live keybind states
        parallax::UpdateKeybindState(boxKey);
        parallax::UpdateKeybindState(aimKey);
        parallax::UpdateKeybindState(triggerKey);
        parallax::UpdateKeybindState(glowKey);

        // Draw background guidance text when menu is closed
        if (!menuOpen) {
            ImGui::SetNextWindowPos(ImVec2(30.0f, 30.0f));
            ImGui::Begin("##ParallaxBackgroundHint", nullptr,
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
            ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 0.9f), "Press [Right Shift] or [Insert] to open Parallax Menu");
            ImGui::End();
        }

        // Render Parallax Window
        if (parallax::BeginWindow("Parallax UI Hub", &menuOpen)) {
            const float dpi = parallax::GetDpiScale();
            const auto& metrics = parallax::Metrics();
            const auto& colors = parallax::Colors();
            const ImVec2 winPos = ImGui::GetWindowPos();
            const ImVec2 winSize = ImGui::GetWindowSize();
            const auto navSide = parallax::GetNavSide();

            // -----------------------------------------------------------------
            // Navigation Bar
            // -----------------------------------------------------------------
            parallax::BeginNavBar();
            if (parallax::NavTab("sliders", "Chained Rows", activeMainTab == 0, "CHAIN")) activeMainTab = 0;
            if (parallax::NavTab("layers", "Selectors", activeMainTab == 1)) activeMainTab = 1;
            if (parallax::NavTab("folder", "Collapsible", activeMainTab == 2)) activeMainTab = 2;
            if (parallax::NavTab("shield", "Safety & Core", activeMainTab == 3)) activeMainTab = 3;
            if (parallax::NavTab("image", "Icon Atlas", activeMainTab == 4)) activeMainTab = 4;

            // User Profile Card in vertical layout docked at bottom
            if (navSide == parallax::NavSide::Left || navSide == parallax::NavSide::Right) {
                const float profileY = winPos.y + winSize.y - (metrics.ProfileCardHeight + 16.0f) * dpi;
                float profileX = winPos.x + 8.0f * dpi;
                if (navSide == parallax::NavSide::Right) {
                    profileX = winPos.x + winSize.x - (metrics.NavWidth - 8.0f) * dpi;
                }
                ImGui::SetCursorScreenPos(ImVec2(profileX, profileY));
                parallax::UserProfileCard(parallax::Context::Get().Profile);
            }
            parallax::EndNavBar();

            // -----------------------------------------------------------------
            // Content Area Layout Calculations
            // -----------------------------------------------------------------
            ImVec2 contentPos;
            ImVec2 contentSize;

            switch (navSide) {
                case parallax::NavSide::Left:
                    contentPos = ImVec2(winPos.x + metrics.NavWidth * dpi + 16.0f * dpi, winPos.y + 16.0f * dpi);
                    contentSize = ImVec2(winSize.x - metrics.NavWidth * dpi - 32.0f * dpi, winSize.y - 32.0f * dpi);
                    break;
                case parallax::NavSide::Right:
                    contentPos = ImVec2(winPos.x + 16.0f * dpi, winPos.y + 16.0f * dpi);
                    contentSize = ImVec2(winSize.x - metrics.NavWidth * dpi - 32.0f * dpi, winSize.y - 32.0f * dpi);
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
            ImGui::BeginChild("##ParallaxMainContentRegion", contentSize, false, ImGuiWindowFlags_NoScrollbar);

            // -----------------------------------------------------------------
            // Top Bar: Search Bar & Header Controls
            // -----------------------------------------------------------------
            ImGui::BeginGroup();
            parallax::SearchBar(searchQuery, "Search widgets, options, keybinds...", 250.0f * dpi);
            ImGui::SameLine(0.0f, 16.0f * dpi);

            ImGui::TextColored(colors.TextMuted, "Dock:");
            ImGui::SameLine(0.0f, 6.0f * dpi);
            auto curSide = parallax::GetNavSide();
            parallax::layout::HeaderWidgets::NavSideSelector(curSide);
            parallax::SetNavSide(curSide);

            ImGui::SameLine(0.0f, 16.0f * dpi);
            ImGui::TextColored(colors.TextMuted, "Size:");
            ImGui::SameLine(0.0f, 6.0f * dpi);
            auto curTier = parallax::GetWindowSizeTier();
            parallax::layout::HeaderWidgets::WindowSizeSelector(curTier);
            parallax::SetWindowSizeTier(curTier);
            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Spacing();

            // -----------------------------------------------------------------
            // Tab 0: Chained Rows (Declarative API Showcase)
            // -----------------------------------------------------------------
            if (activeMainTab == 0) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Declarative Chained Rows
                parallax::BeginGroupbox("Declarative Chained Settings", "sliders");
                ImGui::TextColored(colors.TextMuted, "Rayfield-style chained controls in a single unified row:");
                ImGui::Spacing();

                // 1. Pill Switch with Icon, Description, Tooltip, Keybind, Color Picker, and OnChange
                parallax::Row("Bounding Box Overlay", &boxEsp)
                    .Icon("eye")
                    .Description("Draw crisp dynamic 2D/3D boxes around targets")
                    .Tooltip("Supports customizable color swatch and hotkey trigger")
                    .Keybind(&boxKey)
                    .Color(&boxColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Bounding Box toggled: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 2. Checkbox with Icon, Keybind, Color, Description
                parallax::Row("Snaplines", &snaplines)
                    .Style(parallax::ToggleStyle::Checkbox)
                    .Icon("navigation")
                    .Description("Draw directional guidance line from crosshair")
                    .Tooltip("Line drawn directly to target anchor point")
                    .Color(&snaplineColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Snaplines toggled: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 3. Skeletons with Keybind and Color
                parallax::Row("Skeleton Bones", &skeletons)
                    .Icon("activity")
                    .Description("Overlay anatomical skeletal joint structures")
                    .Color(&skeletonColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Skeletons toggled: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 4. Glow Effect with Keybind & Color
                parallax::Row("Outer Surface Glow", &glowEffect)
                    .Icon("zap")
                    .Description("Harmonic edge outline shader effect")
                    .Keybind(&glowKey)
                    .Color(&glowColor)
                    .OnChange([&](bool val) {
                        logEvent(std::string("Outer Glow toggled: ") + (val ? "ENABLED" : "DISABLED"));
                    });

                // 5. Disabled State Row Showcase
                parallax::Row("Anti-Aliased Depth (Locked)", &disabledDemo)
                    .Icon("lock")
                    .Description("Hardware feature unavailable on this rendering pass")
                    .Disabled(true);

                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Keybind State & Live Activity Stream
                parallax::BeginGroupbox("Live Keybind Status", "keyboard");
                ImGui::TextColored(colors.TextMuted, "Evaluated in real-time with Hold/Toggle/AlwaysOn modes:");
                ImGui::Spacing();

                auto displayKeyStatus = [&](const char* name, const parallax::Keybind& bind) {
                    ImGui::TextColored(colors.TextSecondary, "%s:", name);
                    ImGui::SameLine(160.0f * dpi);
                    ImGui::TextColored(colors.Accent, "[%s]", parallax::GetKeyName(bind.vkey).c_str());
                    ImGui::SameLine(240.0f * dpi);
                    const char* modeStr = bind.mode == parallax::KeybindMode::Hold ? "Hold" :
                                          bind.mode == parallax::KeybindMode::Toggle ? "Toggle" : "AlwaysOn";
                    ImGui::TextColored(colors.TextMuted, "(%s)", modeStr);
                    ImGui::SameLine(330.0f * dpi);
                    ImGui::TextColored(bind.active ? colors.Success : colors.TextMuted,
                        bind.active ? "● ACTIVE" : "○ INACTIVE");
                };

                displayKeyStatus("Bounding Box Key", boxKey);
                displayKeyStatus("Aim Assist Trigger", aimKey);
                displayKeyStatus("Triggerbot Hold", triggerKey);
                displayKeyStatus("Outer Glow Key", glowKey);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Interactive Activity Stream:");
                ImGui::Spacing();

                ImGui::BeginChild("##DemoActivityLog", ImVec2(0.0f, 130.0f * dpi), true, ImGuiWindowFlags_NoScrollbar);
                for (const auto& logMsg : activityLog) {
                    ImGui::TextColored(colors.TextMuted, "• %s", logMsg.c_str());
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

                // Column 1: Smooth Sliders & Dual-Thumb Range Sliders
                parallax::BeginGroupbox("Smooth Spring Sliders", "sliders");
                ImGui::TextColored(colors.TextMuted, "Frame-rate independent spring thumb animation & floating tooltips:");
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
                parallax::BeginGroupbox("Dropdown Selectors", "layers");
                ImGui::TextColored(colors.TextMuted, "Search filtering, SVG icons, and selection badges:");
                ImGui::Spacing();

                // Single Select Dropdown
                if (parallax::Dropdown("Target Hitbox", hitboxItems, &selectedHitbox, hitboxIcons)) {
                    logEvent(std::string("Hitbox changed to: ") + hitboxItems[selectedHitbox]);
                }

                ImGui::Spacing();
                ImGui::Spacing();

                // Searchable Multi-Select Dropdown
                if (parallax::MultiDropdown("Active ESP Indicators", visualElements, selectedElements, visualIcons)) {
                    int activeCount = 0;
                    for (bool b : selectedElements) if (b) activeCount++;
                    logEvent(std::string("Active ESP Indicators updated: ") + std::to_string(activeCount) + " enabled");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextMuted, "Standalone Color Swatch:");
                ImGui::Spacing();
                ImGui::Text("Interface Theme Accent:");
                ImGui::SameLine(180.0f * dpi);
                static ImVec4 customThemeColor{ 0.38f, 0.58f, 0.98f, 1.0f };
                parallax::ColorPicker("##StandaloneAccentColor", &customThemeColor, true);

                ImGui::SameLine(220.0f * dpi);
                ImGui::TextColored(customThemeColor, "HEX: #%02X%02X%02X",
                    (int)(customThemeColor.x * 255.0f),
                    (int)(customThemeColor.y * 255.0f),
                    (int)(customThemeColor.z * 255.0f));

                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 2: Collapsible Groupboxes
            // -----------------------------------------------------------------
            else if (activeMainTab == 2) {
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
                    static float soundVolume = 75.0f;
                    parallax::SliderFloat("Feedback Volume", &soundVolume, 0.0f, 100.0f, "%.0f%%");

                    parallax::EndCollapsibleGroupbox();
                }

                parallax::NextColumn();

                // Column 2: Left-Aligned Header with Icon
                if (parallax::BeginCollapsibleGroupbox("Advanced Computation Engine", &advancedGroupExpanded, "cpu", 0.0f, false)) {
                    ImGui::TextColored(colors.TextMuted, "High-precision calculations and predictive trajectory solver.");
                    ImGui::Spacing();

                    static bool predictPhysics = true;
                    static float predictionTicks = 12.0f;
                    parallax::Row("Predictive Trajectory", &predictPhysics).Icon("zap");
                    parallax::SliderFloat("Prediction Ticks", &predictionTicks, 1.0f, 32.0f, "%.0f ticks");

                    parallax::EndCollapsibleGroupbox();
                }

                parallax::BeginGroupbox("Layout Grid Guide", "layers");
                ImGui::TextWrapped("Parallax responsive grid automatically partitions available content width into equal columns with synchronized spacing.");
                ImGui::Spacing();
                ImGui::BulletText("Single Column (1): Full-width detailed forms");
                ImGui::BulletText("Two Columns (2): Optimal settings & property inspection");
                ImGui::BulletText("Three Columns (3): High-density dashboards & metrics");
                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 3: Safety & Core Architecture
            // -----------------------------------------------------------------
            else if (activeMainTab == 3) {
                parallax::BeginGrid(2, 14.0f);

                // Column 1: Typing Safety & Key Capture Safety
                parallax::BeginGroupbox("Input Safety & Protected Hotkeys", "keyboard");
                ImGui::TextWrapped("Test typing safety below: focus the input text box and type using Right Shift. Notice the menu remains safely open!");
                ImGui::Spacing();

                char inputBuf[128];
                strncpy_s(inputBuf, sizeof(inputBuf), typingTestInput.c_str(), _TRUNCATE);
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputText("##TypingSafetyTest", inputBuf, sizeof(inputBuf))) {
                    typingTestInput = inputBuf;
                }
                ImGui::Spacing();

                bool wantText = ImGui::GetIO().WantTextInput;
                ImGui::TextColored(wantText ? colors.Success : colors.TextMuted,
                    "ImGui WantTextInput: %s (Right Shift is %s)",
                    wantText ? "ACTIVE" : "INACTIVE",
                    wantText ? "SUPPRESSED (Typing Protected)" : "ARMED");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Configured Menu Toggle Hotkeys:");
                ImGui::BulletText("Right Shift (VK_RSHIFT) - Typing-protected");
                ImGui::BulletText("Insert (VK_INSERT) - Always active fallback");
                ImGui::BulletText("Keybind Capture - Suppresses menu toggle while binding keys");
                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Core Architecture & Harmonic Spring Engine
                parallax::BeginGroupbox("Core Architecture & Motion", "cpu");
                ImGui::TextColored(colors.TextSecondary, "DirectX 11 Device: Feature Level 11.0 Hardware");
                ImGui::TextColored(colors.TextSecondary, "DPI Scale: %.2fx", dpi);
                ImGui::TextColored(colors.TextSecondary, "Framerate: %.1f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

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

                ImGui::TextColored(colors.TextSecondary, "Interactive Harmonic Spring Tester:");
                static float testSliderTarget = 50.0f;
                parallax::SliderFloat("Target Position", &testSliderTarget, 0.0f, 100.0f, "%.1f");

                float currentSpringVal = parallax::SpringFloat(ImGui::GetID("##DemoSpringPhysicsVal"), testSliderTarget, {100.0f, 20.0f, 1.0f});
                ImGui::ProgressBar(currentSpringVal / 100.0f, ImVec2(-1.0f, 18.0f * dpi), "");
                ImGui::TextColored(colors.TextMuted, "Settled Physics Position: %.2f", currentSpringVal);

                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            // -----------------------------------------------------------------
            // Tab 4: SVG Icon Atlas
            // -----------------------------------------------------------------
            else if (activeMainTab == 4) {
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
                    ImGui::TextColored(colors.TextMuted, "%s", showcaseIcons[i]);
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
