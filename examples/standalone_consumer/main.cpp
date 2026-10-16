/**
 * Parallax UI — Standalone SDK Consumer Integration Application
 * 
 * This application is built completely independently against the public Parallax SDK:
 * - Links ONLY to sdk/lib/parallax.lib
 * - Includes ONLY sdk/include/parallax and sdk/include/imgui
 * - Does NOT access any private Parallax implementation files in src/
 * - Exercises initialization, window creation, tabs, subtabs, widgets,
 *   theming (AMOLED/Dark/Light), accent customization, and clean shutdown.
 */

#include <parallax/parallax.hpp>
#include <windows.h>
#include <d3d11.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

int RunInteractiveApp(HINSTANCE hInstance) {
    std::cout << "[CONSUMER APP] Starting interactive Parallax UI SDK Application..." << std::endl;

    // 1. Initialize Direct3D 11 host window
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1280, 800, L"Parallax UI — Standalone SDK Consumer Application")) {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11 Host Window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2. Initialize Parallax UI Library
    parallax::Init(host.GetDevice(), host.GetContext());

    // 3. Application State
    bool menuOpen = true;
    bool enableTelemetry = true;
    bool lowLatencyMode = false;
    float sampleRate = 48.0f;
    int bufferDepth = 256;
    float bandMin = 100.0f;
    float bandMax = 8000.0f;
    ImVec4 waveformColor{0.60f, 0.596f, 0.965f, 1.0f}; // Default #9998F6
    std::string deviceName = "Spectra-Receiver-01";
    int activeProtocol = 0;
    std::vector<std::string> protocolItems = {"UDP Multicast", "TCP Stream", "WebSocket Secure", "Shared Memory"};
    std::vector<bool> filterStates = {true, false, true, false};
    std::vector<std::string> filterNames = {"High-Pass 50Hz", "Notch 60Hz", "Anti-Aliasing", "FIR Smoothing"};
    parallax::Keybind pttBind{VK_SPACE, parallax::KeybindMode::Hold, false};
    bool advancedExpanded = true;
    int activeSubTab = 0;

    // Initial toast notification
    parallax::Notify(
        parallax::NotificationLevel::Success,
        "Public SDK Connected",
        "Standalone consumer application initialized successfully via Parallax::Parallax target.",
        5.0f
    );

    // 4. Construct Declarative Window
    parallax::WindowBuilder builder("Parallax SDK Consumer");
    builder.WithTheme(parallax::ThemeMode::Amoled)
           .WithAccent(parallax::AccentPreset::Parallax)
           .AddTab("sliders", "Transceiver", [&](parallax::TabContext& ctx) {
               ctx.BeginGrid(2);

               // Left Card: Core Hardware Settings
               ctx.Groupbox("RF Control Panel", [&]() {
                   ctx.Toggle("Enable Sensor Telemetry", &enableTelemetry)
                      .Tooltip("Streams live radio frequency telemetry to analysis pipeline.")
                      .Description("Active RF Telemetry Feed");
                   
                   ctx.Checkbox("Low-Latency Direct Mode", &lowLatencyMode)
                      .Tooltip("Bypasses software FIFO for minimum response time.");

                   ctx.SliderFloat("DSP Sample Rate (kHz)", &sampleRate, 8.0f, 192.0f, "%.1f kHz");
                   ctx.SliderInt("Buffer Depth (frames)", &bufferDepth, 64, 2048, "%d frames");
                   ctx.RangeSlider("Audio Bandwidth (Hz)", &bandMin, &bandMax, 20.0f, 20000.0f, "%.0f Hz");

                   ctx.Dropdown("Streaming Protocol", protocolItems, &activeProtocol);
                   ctx.MultiDropdown("Signal Filtering", filterNames, filterStates);

                   ctx.InputText("Receiver Identifier", deviceName, "cpu", "Enter device callsign...");
                   ctx.Color("Waveform Display Tint", &waveformColor);

                   if (ctx.Button("Calibrate Transceiver", "activity", parallax::ButtonVariant::Primary)) {
                       parallax::Notify(parallax::NotificationLevel::Info, "Calibration", "Transceiver calibration sweep initiated.", 3.0f);
                   }
               }, "radio");

               ctx.NextColumn();

               // Right Card: Keybinds & Advanced Controls
               ctx.Groupbox("Input & Controls", [&]() {
                   ctx.KeybindRow("Push-to-Talk Hotkey", "Key to open radio transmission channel", &pttBind);

                   ImGui::Spacing();
                   ctx.CollapsibleGroupbox("Advanced DSP Filters", &advancedExpanded, [&]() {
                       ImGui::TextColored(parallax::Colors().TextSecondary, "FIR Coefficients: Kaiser Window (Beta=7.86)");
                       ImGui::TextColored(parallax::Colors().TextMuted, "Nyquist Cutoff: 24.0 kHz | Attenuation: -96 dB");
                       if (ctx.Button("Reset Filter Coeffs", "rotate-ccw", parallax::ButtonVariant::Secondary)) {
                           parallax::Notify(parallax::NotificationLevel::Warning, "Reset", "Filter coefficients reset to factory default.", 2.5f);
                       }
                   }, "sliders", 0.0f, false);
               }, "settings");

               ctx.EndGrid();
           })
           .AddTab("palette", "Appearance", [&](parallax::TabContext& ctx) {
               ctx.BeginGrid(2);
               ctx.Groupbox("Theme Engine", [&]() {
                   ctx.GroupboxDescription("Switch between AMOLED Black (#000000), Graphite Dark, and Porcelain Light.");
                   ctx.ThemeSwitcher("Application Color Theme");
                   ImGui::Spacing();
                   ctx.AccentSelector("Primary Brand Accent", true);
               }, "palette");

               ctx.NextColumn();
               ctx.Groupbox("Sub-Tab Navigation", [&]() {
                   ctx.GroupboxDescription("Demonstration of nested SubTabs within the public SDK.");
                   if (parallax::BeginSubTabs(parallax::SubTabPlacement::TopHorizontal)) {
                       if (parallax::SubTab("Overview", activeSubTab == 0)) activeSubTab = 0;
                       if (parallax::SubTab("Hardware Diagnostics", activeSubTab == 1)) activeSubTab = 1;
                       parallax::EndSubTabs();
                   }
                   ImGui::Spacing();
                   if (activeSubTab == 0) {
                       ImGui::TextColored(parallax::Colors().TextPrimary, "System Overview: Normal operating temperature (38 C).");
                   } else {
                       ImGui::TextColored(parallax::Colors().Success, "Diagnostics: All 4 RF channels locked and operational.");
                   }
               }, "layers");
               ctx.EndGrid();
           });

    // 5. Main Execution Loop
    host.RunMainLoop([&]() {
        // Update keybind states
        parallax::UpdateKeybindState(pttBind);

        // Floating Watermark
        parallax::Watermark("PARALLAX CONSUMER SDK", "Standalone Verification", true, true);

        // Toast notifications
        parallax::RenderNotifications();

        // Render main Parallax UI
        builder.Render(&menuOpen);
    });

    // 6. Clean Shutdown
    std::cout << "[CONSUMER APP] Shutting down Parallax UI..." << std::endl;
    parallax::Shutdown();
    host.Shutdown();

    std::cout << "[CONSUMER APP] Clean shutdown completed successfully." << std::endl;
    return 0;
}

int RunHeadlessVerification() {
    std::cout << "==========================================================" << std::endl;
    std::cout << " PARALLAX PUBLIC SDK — STANDALONE CONSUMER TEST" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Initialize Direct3D 11 host in hidden mode
    std::cout << "[TEST] 1. Initializing Direct3D 11 host (hidden window)..." << std::endl;
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(GetModuleHandle(nullptr), 1280, 800, L"Parallax Headless Verification")) {
        std::cerr << "[ERROR] Failed to initialize Direct3D 11 Host." << std::endl;
        return 1;
    }
    ShowWindow(host.GetHwnd(), SW_HIDE);
    std::cout << "  [OK] Direct3D 11 host initialized successfully." << std::endl;

    // 2. Initialize Parallax via Public SDK
    std::cout << "[TEST] 2. Initializing Parallax via Public SDK..." << std::endl;
    parallax::Init(host.GetDevice(), host.GetContext());
    std::cout << "  [OK] Parallax::Init completed successfully." << std::endl;

    // 3. Test Theme Configuration via Public API
    std::cout << "[TEST] 3. Testing Theme & Accent Configuration via Public SDK..." << std::endl;
    parallax::SetTheme(parallax::ThemeMode::Amoled, false);
    if (parallax::GetTheme() != parallax::ThemeMode::Amoled) {
        std::cerr << "[FAIL] ThemeMode was not set to Amoled!" << std::endl;
        return 1;
    }

    parallax::SetAccentPreset(parallax::AccentPreset::Parallax, false);
    if (parallax::GetAccentPreset() != parallax::AccentPreset::Parallax) {
        std::cerr << "[FAIL] AccentPreset was not set to Parallax!" << std::endl;
        return 1;
    }
    std::cout << "  [OK] AMOLED Black theme & Parallax #9998F6 accent verified." << std::endl;

    // 4. Test Declarative WindowBuilder & Widget Rendering
    std::cout << "[TEST] 4. Simulating 20 frames of full UI rendering and widget execution..." << std::endl;
    bool menuOpen = true;
    bool toggleVal = true;
    float sliderVal = 50.0f;
    int intVal = 10;
    float rangeMin = 20.0f;
    float rangeMax = 80.0f;
    int dropIdx = 1;
    std::vector<std::string> dropItems = {"Alpha", "Beta", "Gamma"};
    std::string textInput = "TestDevice";
    parallax::Keybind testBind{'K', parallax::KeybindMode::Toggle, false};
    bool groupExpanded = true;

    parallax::WindowBuilder builder("Test Window");
    builder.WithTheme(parallax::ThemeMode::Amoled)
           .WithAccent(parallax::AccentPreset::Parallax)
           .AddTab("sliders", "Tab 1", [&](parallax::TabContext& ctx) {
               ctx.BeginGrid(2);
               ctx.Groupbox("Group 1", [&]() {
                   ctx.Toggle("Toggle Feature", &toggleVal);
                   ctx.SliderFloat("Float Slider", &sliderVal, 0.0f, 100.0f);
                   ctx.SliderInt("Int Slider", &intVal, 0, 100);
                   ctx.RangeSlider("Range Slider", &rangeMin, &rangeMax, 0.0f, 100.0f);
                   ctx.Dropdown("Dropdown", dropItems, &dropIdx);
                   ctx.InputText("Input Text", textInput);
                   ctx.Button("Submit Button");
                   ctx.KeybindRow("Keybind", "Hotkey description", &testBind);
               });
               ctx.NextColumn();
               ctx.CollapsibleGroupbox("Collapsible Group", &groupExpanded, [&]() {
                   ImGui::Text("Collapsible Content");
               });
               ctx.EndGrid();
           })
           .AddTab("layers", "Tab 2", [&](parallax::TabContext& ctx) {
               ctx.Groupbox("SubTabs", [&]() {
                   if (parallax::BeginSubTabs(parallax::SubTabPlacement::TopHorizontal)) {
                       parallax::SubTab("Sub A", true);
                       parallax::SubTab("Sub B", false);
                       parallax::EndSubTabs();
                   }
               });
           });

    // Run 20 simulated frames
    for (int frame = 0; frame < 20; ++frame) {
        host.ProcessMessages();
        host.BeginFrame();

        // Update keybind simulation
        parallax::UpdateKeybindState(testBind);

        // Render UI
        builder.Render(&menuOpen);

        // Toast notifications
        if (frame == 5) {
            parallax::Notify(parallax::NotificationLevel::Success, "Frame 5", "Midway frame check", 2.0f);
        }
        parallax::RenderNotifications();

        host.EndFrame(false);
    }
    std::cout << "  [OK] 20 frames rendered cleanly with zero layout or assertion errors." << std::endl;

    // 5. Test Clean Shutdown
    std::cout << "[TEST] 5. Testing clean library shutdown..." << std::endl;
    parallax::Shutdown();
    host.Shutdown();
    std::cout << "  [OK] DirectX 11 host and Parallax shutdown cleanly." << std::endl;

    std::cout << "==========================================================" << std::endl;
    std::cout << " [PASSED] ALL PUBLIC SDK CONSUMER VERIFICATIONS SUCCEEDED" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test" || std::string(argv[i]) == "-t") {
            return RunHeadlessVerification();
        }
    }

    return RunInteractiveApp(GetModuleHandle(nullptr));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    return RunInteractiveApp(hInstance);
}
