#include "parallax/parallax.hpp"
#include <windows.h>
#include <string>

int RunDemo(HINSTANCE hInstance) {
    // 1. Initialize standalone DirectX 11 Host window
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1280, 720, L"Parallax UI DX11 Demo")) {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 Host Window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2. Initialize Parallax UI Library
    parallax::Init(host.GetDevice(), host.GetContext());

    // 3. Demo state
    bool menuOpen = true;
    int activeMainTab = 0;
    int activeSubTab = 0;
    std::string searchQuery = "";
    std::string typingTestInput = "Type here with Right Shift...";

    // Mock settings
    bool enableAntialiasing = true;
    bool enableVsync = true;
    bool enableHdrGlow = false;
    float masterVolume = 85.0f;

    // 4. Run main message and render loop
    host.RunMainLoop([&]() {
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
        if (parallax::BeginWindow("Parallax Control Hub", &menuOpen)) {
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
            if (parallax::NavTab("shield", "Security", activeMainTab == 0)) activeMainTab = 0;
            if (parallax::NavTab("activity", "Dashboard", activeMainTab == 1, "NEW")) activeMainTab = 1;
            if (parallax::NavTab("sliders", "Visuals", activeMainTab == 2)) activeMainTab = 2;
            if (parallax::NavTab("cpu", "Engine", activeMainTab == 3)) activeMainTab = 3;
            if (parallax::NavTab("palette", "Theming", activeMainTab == 4)) activeMainTab = 4;

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
            // Top Bar inside Content: Search Bar & Header Controls
            // -----------------------------------------------------------------
            ImGui::BeginGroup();
            parallax::SearchBar(searchQuery, "Search settings, modules, icons...", 240.0f * dpi);
            ImGui::SameLine(0.0f, 16.0f * dpi);

            ImGui::TextColored(colors.TextMuted, "Nav Side:");
            ImGui::SameLine(0.0f, 6.0f * dpi);
            auto curSide = parallax::GetNavSide();
            parallax::layout::HeaderWidgets::NavSideSelector(curSide);
            parallax::SetNavSide(curSide);

            ImGui::SameLine(0.0f, 16.0f * dpi);
            ImGui::TextColored(colors.TextMuted, "Window Tier:");
            ImGui::SameLine(0.0f, 6.0f * dpi);
            auto curTier = parallax::GetWindowSizeTier();
            parallax::layout::HeaderWidgets::WindowSizeSelector(curTier);
            parallax::SetWindowSizeTier(curTier);
            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Spacing();

            // -----------------------------------------------------------------
            // Sub-Tabs with Harmonic Stretching Active Indicator
            // -----------------------------------------------------------------
            parallax::BeginSubTabs(parallax::SubTabPlacement::TopHorizontal);
            if (parallax::SubTab("Overview & Status", activeSubTab == 0)) activeSubTab = 0;
            if (parallax::SubTab("Layout & Docking", activeSubTab == 1)) activeSubTab = 1;
            if (parallax::SubTab("Motion & Physics", activeSubTab == 2)) activeSubTab = 2;
            if (parallax::SubTab("SVG Icon Gallery", activeSubTab == 3)) activeSubTab = 3;
            parallax::EndSubTabs();

            ImGui::Spacing();

            // -----------------------------------------------------------------
            // Tab Content: Responsive 1/2/3-Column Grids & Groupboxes
            // -----------------------------------------------------------------
            if (activeSubTab == 0) {
                // Overview & Status: 2-column grid
                parallax::BeginGrid(2, 14.0f);

                // Column 1: System Status & Typing Safety Verification
                parallax::BeginGroupbox("Typing Safety & Input Verification", "keyboard");
                ImGui::TextWrapped("Test typing safety below: focus the input and type using Right Shift. Notice the menu does NOT close!");
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
                    wantText ? "SUPPRESSED (Safe)" : "ARMED");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(colors.TextSecondary, "Toggle Hotkeys:");
                ImGui::BulletText("Right Shift (VK_RSHIFT) - typing protected");
                ImGui::BulletText("Insert (VK_INSERT) - always active");
                parallax::EndGroupbox();

                parallax::NextColumn();

                // Column 2: Core Architecture Health
                parallax::BeginGroupbox("Core Architecture Health", "cpu");
                ImGui::TextColored(colors.TextSecondary, "DirectX 11 Device: Hardware (Feature 11.0)");
                ImGui::TextColored(colors.TextSecondary, "Display DPI Scale: %.2fx", dpi);
                ImGui::TextColored(colors.TextSecondary, "Framerate: %.1f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

                ImGui::Spacing();
                ImGui::Checkbox("Enable Anti-Aliasing", &enableAntialiasing);
                ImGui::Checkbox("Enable Vertical Sync", &enableVsync);
                ImGui::Checkbox("Enable Subtle Edge Glow", &enableHdrGlow);

                ImGui::Spacing();
                ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 100.0f, "%.0f%%");
                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            else if (activeSubTab == 1) {
                // Layout & Docking: 3-column grid
                parallax::BeginGrid(3, 12.0f);

                parallax::BeginGroupbox("Navigation Dock", "layers");
                ImGui::TextWrapped("Switch navigation bar dock side at runtime:");
                ImGui::Spacing();
                if (ImGui::RadioButton("Left Dock", curSide == parallax::NavSide::Left)) parallax::SetNavSide(parallax::NavSide::Left);
                if (ImGui::RadioButton("Right Dock", curSide == parallax::NavSide::Right)) parallax::SetNavSide(parallax::NavSide::Right);
                if (ImGui::RadioButton("Top Dock", curSide == parallax::NavSide::Top)) parallax::SetNavSide(parallax::NavSide::Top);
                if (ImGui::RadioButton("Bottom Dock", curSide == parallax::NavSide::Bottom)) parallax::SetNavSide(parallax::NavSide::Bottom);
                parallax::EndGroupbox();

                parallax::NextColumn();

                parallax::BeginGroupbox("Window Size Tier", "maximize");
                ImGui::TextWrapped("Smooth spring-animated window dimensions:");
                ImGui::Spacing();
                if (ImGui::RadioButton("Compact (720x480)", curTier == parallax::WindowSizeTier::Compact)) parallax::SetWindowSizeTier(parallax::WindowSizeTier::Compact);
                if (ImGui::RadioButton("Standard (900x600)", curTier == parallax::WindowSizeTier::Standard)) parallax::SetWindowSizeTier(parallax::WindowSizeTier::Standard);
                if (ImGui::RadioButton("Expanded (1080x720)", curTier == parallax::WindowSizeTier::Expanded)) parallax::SetWindowSizeTier(parallax::WindowSizeTier::Expanded);
                parallax::EndGroupbox();

                parallax::NextColumn();

                parallax::BeginGroupbox("Corner Drag Bounds", "move");
                ImGui::TextWrapped("You can also smoothly resize this window by dragging the bottom-right corner.");
                ImGui::Spacing();
                ImGui::BulletText("Minimum Width: 640px");
                ImGui::BulletText("Minimum Height: 440px");
                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            else if (activeSubTab == 2) {
                // Motion & Physics: 2-column grid
                parallax::BeginGrid(2, 14.0f);

                parallax::BeginGroupbox("Harmonic Spring Physics", "zap");
                ImGui::TextWrapped("Parallax uses analytical damped harmonic spring physics (k=100, c=20) for tactile, weighty micro-interactions.");
                ImGui::Spacing();

                static float testSliderTarget = 50.0f;
                ImGui::SliderFloat("Target Value", &testSliderTarget, 0.0f, 100.0f);

                float currentSpringVal = parallax::SpringFloat(ImGui::GetID("##DemoSpringPhysicsVal"), testSliderTarget, {100.0f, 20.0f, 1.0f});
                ImGui::ProgressBar(currentSpringVal / 100.0f, ImVec2(-1.0f, 20.0f * dpi), "");
                ImGui::TextColored(colors.TextMuted, "Physically Settled Value: %.2f", currentSpringVal);
                parallax::EndGroupbox();

                parallax::NextColumn();

                parallax::BeginGroupbox("Animation Speed Multiplier", "clock");
                ImGui::TextWrapped("Global animation scale controls transition speeds across all components:");
                ImGui::Spacing();

                auto speed = parallax::GetSpeedMultiplier();
                if (ImGui::RadioButton("Fast (1.5x)", speed == parallax::SpeedMultiplier::Fast)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Fast);
                if (ImGui::RadioButton("Normal (1.0x)", speed == parallax::SpeedMultiplier::Normal)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Normal);
                if (ImGui::RadioButton("Relaxed (0.7x)", speed == parallax::SpeedMultiplier::Relaxed)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Relaxed);
                if (ImGui::RadioButton("Off (Reduced Motion)", speed == parallax::SpeedMultiplier::Off)) parallax::SetSpeedMultiplier(parallax::SpeedMultiplier::Off);
                parallax::EndGroupbox();

                parallax::EndGrid();
            }
            else if (activeSubTab == 3) {
                // SVG Icon Gallery: showcase lazy loading and dynamic tinting
                parallax::BeginGroupbox("On-Demand 2,077 SVG Icon Showcase (Dynamic Atlas)", "image");
                ImGui::TextWrapped("Icons are rasterized on first use at display DPI and packed into a single dynamic DirectX 11 Texture Atlas:");
                ImGui::Spacing();

                const char* showcaseIcons[] = {
                    "shield", "activity", "sliders", "palette", "zap", "bell", "cpu", "layers",
                    "user", "search", "check", "folder", "database", "terminal", "lock", "globe",
                    "camera", "wifi", "battery-charging", "cloud", "mail", "flag", "heart", "star"
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
                ImGui::TextColored(colors.TextSecondary, "Missing Icon Fallback Test:");
                parallax::Icon("non_existent_icon_test", ImVec2(24.0f * dpi, 24.0f * dpi), colors.Warning);
                ImGui::SameLine();
                ImGui::TextColored(colors.TextMuted, "Missing icon requests gracefully fall back to the built-in procedural shield with zero crashes.");
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
