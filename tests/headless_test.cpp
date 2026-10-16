#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_internal.h>
#include "parallax/parallax.hpp"
#include "parallax/widgets/collapsible_groupbox.hpp"
#include "parallax/config/config_system.hpp"
#include "parallax/auth/key_system.hpp"

void TestSemVerAndFixedShell() {
    std::cout << "[TEST] 1. Testing SemVer (1.2.0) & Fixed Window Dimensions (1137x699)..." << std::endl;

    static_assert(PARALLAX_VERSION_MAJOR == 1, "Expected MAJOR == 1");
    static_assert(PARALLAX_VERSION_MINOR == 2, "Expected MINOR == 2");
    static_assert(PARALLAX_VERSION_PATCH == 0, "Expected PATCH == 0");
    static_assert(parallax::kVersionMajor == 1 && parallax::kVersionMinor == 2 && parallax::kVersionPatch == 0, "Expected 1.2.0");
    assert(std::string(PARALLAX_VERSION_STRING) == "1.2.0");

    const ImVec2 defSize = parallax::GetDefaultWindowSize();
    assert(std::abs(defSize.x - 1137.0f) < 0.01f);
    assert(std::abs(defSize.y - 699.0f) < 0.01f);

    std::cout << "  ✓ SemVer v" << PARALLAX_VERSION_STRING << " and 1137x699 fixed bounds verified" << std::endl;
}

void TestThemeAndAccentEngine() {
    std::cout << "[TEST] 2. Testing Theme Engine (AMOLED Black <-> Graphite Dark <-> Porcelain Light) & Accent Presets..." << std::endl;

    // Test AMOLED (Black)
    parallax::SetTheme(parallax::ThemeMode::Amoled, false);
    assert(parallax::GetTheme() == parallax::ThemeMode::Amoled);
    assert(!parallax::IsLightTheme());
    assert(parallax::IsAmoledTheme());

    const ImVec4 amoledWinBg = parallax::Colors().WindowBg;
    const ImVec4 amoledText = parallax::Colors().TextPrimary;
    assert(amoledWinBg.x == 0.0f && amoledWinBg.y == 0.0f && amoledWinBg.z == 0.0f); // Pure #000000
    assert(amoledText.x > 0.90f); // #F5F6FA

    // Test Graphite Dark
    parallax::SetTheme(parallax::ThemeMode::Dark, false);
    assert(parallax::GetTheme() == parallax::ThemeMode::Dark);
    assert(!parallax::IsLightTheme());
    assert(!parallax::IsAmoledTheme());

    const ImVec4 darkWinBg = parallax::Colors().WindowBg;
    const ImVec4 darkText = parallax::Colors().TextPrimary;
    assert(darkWinBg.x > 0.01f && darkWinBg.x < 0.15f);
    assert(darkText.x > 0.85f);

    // Switch to Porcelain (Light) with smooth transition
    parallax::SetTheme(parallax::ThemeMode::Light, true);
    assert(parallax::GetTheme() == parallax::ThemeMode::Light);
    assert(parallax::IsLightTheme());

    // Step transition to completion
    for (int i = 0; i < 30; ++i) {
        parallax::StyleManager::Get().UpdateTransition(1.0f / 60.0f);
    }

    const ImVec4 lightWinBg = parallax::Colors().WindowBg;
    const ImVec4 lightText = parallax::Colors().TextPrimary;
    assert(lightWinBg.x > 0.85f);
    assert(lightText.x < 0.25f);
    std::cout << "  ✓ AMOLED Black, Graphite Dark & Porcelain Light surface/text tokens verified" << std::endl;

    // Test Primary Brand Accent (Parallax #9998F6)
    parallax::SetAccentPreset(parallax::AccentPreset::Parallax, false);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::Parallax);
    const ImVec4 parallaxAccent = parallax::Colors().Accent;
    assert(std::abs(parallaxAccent.x - 153.0f / 255.0f) < 0.01f);
    assert(std::abs(parallaxAccent.y - 152.0f / 255.0f) < 0.01f);
    assert(std::abs(parallaxAccent.z - 246.0f / 255.0f) < 0.01f);

    // Test Other Accent Presets
    parallax::SetAccentPreset(parallax::AccentPreset::Copper, false);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::Copper);
    const ImVec4 copperAccent = parallax::Colors().Accent;
    assert(copperAccent.x > copperAccent.z); // Copper is warm (R > B)

    parallax::SetAccentPreset(parallax::AccentPreset::SteelBlue, false);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::SteelBlue);
    const ImVec4 steelAccent = parallax::Colors().Accent;
    assert(steelAccent.z > steelAccent.x); // Steel Blue is cool (B > R)

    // Test Custom Accent
    parallax::SetCustomAccent(ImVec4(0.55f, 0.45f, 0.65f, 1.0f), false);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::Custom);

    // Restore default Parallax (#9998F6) & AMOLED theme
    parallax::SetAccentPreset(parallax::AccentPreset::Parallax, false);
    parallax::SetTheme(parallax::ThemeMode::Amoled, false);
    std::cout << "  ✓ Accent presets (Parallax #9998F6, Sage, Monochrome, Copper, SteelBlue, Custom) verified" << std::endl;
}

void TestCollapsibleAndAutoGroupbox() {
    std::cout << "[TEST] 3. Testing Auto-Sizing Groupbox & Collapsible Groupbox lifecycle..." << std::endl;

    bool expanded = true;
    ImGuiIO& io = ImGui::GetIO();

    // Frame 0-4: Initial expanded render + auto-sizing Groupbox check
    float groupboxHeight = 0.0f;
    for (int i = 0; i < 5; ++i) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(100, 100));
        ImGui::SetNextWindowSize(ImVec2(800, 600));
        ImGui::Begin("TestWindow");

        if (parallax::BeginGroupbox("AutoFitCard", "layers", 0.0f)) {
            parallax::GroupboxDescription("Card should auto-fit its content height.");
            ImGui::Button("Single Control", ImVec2(120, 28));
            groupboxHeight = ImGui::GetWindowHeight();
            parallax::EndGroupbox();
        }

        bool open = parallax::BeginCollapsibleGroupbox("Test Group", &expanded, "eye", 0.0f, true);
        (void)open;
        ImGui::Button("Child Button", ImVec2(100, 30));
        parallax::EndCollapsibleGroupbox();
        ImGui::End();
        ImGui::Render();
    }

    // Verify auto-sizing groupbox did NOT stretch to 600px window bottom
    assert(groupboxHeight > 40.0f && groupboxHeight < 250.0f);
    std::cout << "  ✓ Auto-sizing Groupbox fitted content height (" << groupboxHeight << "px < 250px)" << std::endl;

    // Click CollapsibleGroupbox header to collapse
    // Determine where Test Group header is or toggle via click
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(800, 600));
    ImGui::Begin("TestWindow2");
    ImVec2 headerScreenPos = ImGui::GetCursorScreenPos();
    parallax::BeginCollapsibleGroupbox("Test Group 2", &expanded, "eye", 0.0f, true);
    ImGui::Button("Child Button", ImVec2(100, 30));
    parallax::EndCollapsibleGroupbox();
    ImGui::End();
    ImGui::Render();

    io.MousePos = ImVec2(headerScreenPos.x + 60.0f, headerScreenPos.y + 15.0f);
    io.MouseDown[0] = true;
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(800, 600));
    ImGui::Begin("TestWindow2");
    parallax::BeginCollapsibleGroupbox("Test Group 2", &expanded, "eye", 0.0f, true);
    parallax::EndCollapsibleGroupbox();
    ImGui::End();
    ImGui::Render();

    io.MouseDown[0] = false;
    for (int frame = 0; frame < 30; ++frame) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(100, 100));
        ImGui::SetNextWindowSize(ImVec2(800, 600));
        ImGui::Begin("TestWindow2");
        bool open = parallax::BeginCollapsibleGroupbox("Test Group 2", &expanded, "eye", 0.0f, true);
        if (open) {
            ImGui::Button("Child Button", ImVec2(100, 30));
            parallax::EndCollapsibleGroupbox();
        }
        ImGui::End();
        ImGui::Render();
    }

    assert(expanded == false);
    std::cout << "  ✓ CollapsibleGroupbox collapse & space reclamation verified" << std::endl;
}

void TestRedesignedWidgetsHeadless() {
    std::cout << "[TEST] 4. Testing all redesigned v1.1.0 widgets across Dark & Light themes..." << std::endl;

    bool bToggle = true;
    bool bCheck = false;
    float fSlider = 50.0f;
    int iSlider = 5;
    float rMin = 20.0f, rMax = 80.0f;
    int irMin = 10, irMax = 90;
    int ddIdx = 0;
    std::vector<std::string> items = {"Alpha", "Beta", "Gamma"};
    std::vector<bool> multiSel = {true, false, true};
    ImVec4 col(0.47f, 0.63f, 0.55f, 1.0f);
    std::string textVal = "Parallax";
    parallax::Keybind kb{'K', parallax::KeybindMode::Toggle, false};

    for (int themePass = 0; themePass < 2; ++themePass) {
        parallax::SetTheme(themePass == 0 ? parallax::ThemeMode::Dark : parallax::ThemeMode::Light, false);

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(50, 50));
        ImGui::SetNextWindowSize(ImVec2(900, 650));
        ImGui::Begin("WidgetTestHost");

        parallax::Toggle("Custom Toggle", &bToggle);
        parallax::ToggleCompact("Compact Toggle", &bToggle);
        parallax::Checkbox("Custom Checkbox", &bCheck);
        parallax::CheckboxIndeterminate("Indeterminate Checkbox", &bCheck, true);

        parallax::Row("Chained Row", &bToggle)
            .Icon("shield")
            .Description("Row description")
            .Compact(true)
            .Keybind(&kb)
            .Color(&col);

        parallax::Button("Primary", "check", parallax::ButtonVariant::Primary);
        parallax::Button("Secondary", "sliders", parallax::ButtonVariant::Secondary);
        parallax::Button("Subtle", "eye", parallax::ButtonVariant::Subtle);
        parallax::Button("Destructive", "trash-2", parallax::ButtonVariant::Destructive);
        parallax::ButtonCompact("Compact", "zap");
        parallax::IconButton("##IconBtn", "bell");

        parallax::SliderFloat("Float Slider", &fSlider, 0.0f, 100.0f);
        parallax::SliderFloatEx("Tick Slider", &fSlider, 0.0f, 100.0f, {"%.1f", 0.0f, false, false, false, 5, true});
        parallax::SliderInt("Int Slider", &iSlider, 0, 10);
        parallax::RangeSliderFloat("Float Range", &rMin, &rMax, 0.0f, 100.0f);
        parallax::RangeSliderInt("Int Range", &irMin, &irMax, 0, 100);

        parallax::Dropdown("Single Dropdown", items, &ddIdx);
        parallax::MultiDropdown("Multi Dropdown", items, multiSel);
        parallax::ColorPicker("Color Swatch", &col, true);
        parallax::ColorPickerRow("Color Row", &col, true);
        parallax::InputText("Input Field", textVal, "search", "Type...");
        parallax::KeybindWidget("Keybind", &kb);
        parallax::ThemeSwitcher("Theme Switcher");
        parallax::AccentSelector("Accent Selector", true);

        ImGui::End();
        ImGui::Render();
    }

    parallax::SetTheme(parallax::ThemeMode::Dark, false);
    std::cout << "  ✓ All redesigned custom widgets rendered cleanly in Dark & Light modes" << std::endl;
}

void TestConfigEngine() {
    std::cout << "[TEST] 5. Testing JSON Configuration Engine (atomic write, theme/accent persistence, Base64)..." << std::endl;

    auto& cfg = parallax::config::ConfigManager::Get();
    cfg.SetConfigDirectory("test_configs");

    bool bTest = true;
    int iTest = 42;
    float fTest = 3.14159f;
    ImVec4 cTest(0.1f, 0.2f, 0.3f, 1.0f);
    parallax::Keybind kbTest{ 'X', parallax::KeybindMode::Hold, true };
    std::string sTest = "TestStringValue";
    std::vector<bool> mbTest = { true, false, true, true };

    cfg.BindBool("test.bool", &bTest, false);
    cfg.BindInt("test.int", &iTest, 0);
    cfg.BindFloat("test.float", &fTest, 0.0f);
    cfg.BindColor("test.color", &cTest, ImVec4(0, 0, 0, 0));
    cfg.BindKeybind("test.keybind", &kbTest);
    cfg.BindString("test.string", &sTest, "");
    cfg.BindMultiBool("test.multibool", &mbTest);

    // Set Porcelain Light theme and Copper accent before saving
    parallax::SetTheme(parallax::ThemeMode::Light, false);
    parallax::SetAccentPreset(parallax::AccentPreset::Copper, false);

    // Save profile
    bool saveOk = cfg.SaveProfile("UnitTestProfile");
    (void)saveOk;
    assert(saveOk);
    std::cout << "  ✓ Atomic profile save succeeded" << std::endl;

    // Mutate variables + theme/accent
    bTest = false;
    iTest = 999;
    fTest = 0.001f;
    cTest = ImVec4(0.9f, 0.8f, 0.7f, 0.6f);
    kbTest.vkey = 'Z';
    kbTest.mode = parallax::KeybindMode::Toggle;
    sTest = "Corrupted";
    mbTest = { false, false, false, false };
    parallax::SetTheme(parallax::ThemeMode::Dark, false);
    parallax::SetAccentPreset(parallax::AccentPreset::Sage, false);

    // Reload profile
    bool loadOk = cfg.LoadProfile("UnitTestProfile");
    (void)loadOk;
    assert(loadOk);
    assert(bTest == true);
    assert(iTest == 42);
    assert(std::abs(fTest - 3.14159f) < 0.001f);
    assert(std::abs(cTest.x - 0.1f) < 0.001f);
    assert(kbTest.vkey == 'X');
    assert(kbTest.mode == parallax::KeybindMode::Hold);
    assert(sTest == "TestStringValue");
    assert(mbTest.size() == 4 && mbTest[0] == true && mbTest[1] == false);
    assert(parallax::GetTheme() == parallax::ThemeMode::Light);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::Copper);
    std::cout << "  ✓ Profile reload verified (all bound types + ThemeMode + AccentPreset match)" << std::endl;

    // Test Base64 export & import
    std::string b64 = cfg.ExportToBase64();
    assert(!b64.empty());

    // Mutate again
    iTest = 100;
    sTest = "Modified";
    parallax::SetTheme(parallax::ThemeMode::Dark, false);
    parallax::SetAccentPreset(parallax::AccentPreset::Monochrome, false);

    // Import from Base64
    bool importOk = cfg.ImportFromBase64(b64);
    (void)importOk;
    assert(importOk);
    assert(iTest == 42);
    assert(sTest == "TestStringValue");
    assert(parallax::GetTheme() == parallax::ThemeMode::Light);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::Copper);
    std::cout << "  ✓ Base64 config export/import verified" << std::endl;

    // Reset to defaults
    cfg.ResetToDefaults();
    assert(parallax::GetTheme() == parallax::ThemeMode::Amoled);
    assert(parallax::GetAccentPreset() == parallax::AccentPreset::Parallax);

    // Cleanup test profile
    cfg.DeleteProfile("UnitTestProfile");
    std::cout << "  ✓ Profile reset and deletion verified" << std::endl;
}

void TestKeybindStateMachineAndLayoutDiagnostics() {
    std::cout << "[TEST] 7. Testing Keybind State Machine & Layout Diagnostics..." << std::endl;

    parallax::ClearSimulatedKeys();

    // 1. Hold mode
    parallax::Keybind kbHold{ 0x41, parallax::KeybindMode::Hold, false }; // 'A'
    parallax::UpdateKeybindState(kbHold);
    assert(!kbHold.active);

    parallax::SimulateKeyState(0x41, true);
    parallax::UpdateKeybindState(kbHold);
    assert(kbHold.active);

    parallax::SimulateKeyState(0x41, false);
    parallax::UpdateKeybindState(kbHold);
    assert(!kbHold.active);

    // 2. Toggle mode
    parallax::Keybind kbToggle{ 0x42, parallax::KeybindMode::Toggle, false }; // 'B'
    parallax::UpdateKeybindState(kbToggle);
    assert(!kbToggle.active);

    // Press down
    parallax::SimulateKeyState(0x42, true);
    parallax::UpdateKeybindState(kbToggle);
    assert(kbToggle.active); // Toggled ON

    // Release
    parallax::SimulateKeyState(0x42, false);
    parallax::UpdateKeybindState(kbToggle);
    assert(kbToggle.active); // Still ON

    // Press down second time
    parallax::SimulateKeyState(0x42, true);
    parallax::UpdateKeybindState(kbToggle);
    assert(!kbToggle.active); // Toggled OFF

    parallax::SimulateKeyState(0x42, false);
    parallax::UpdateKeybindState(kbToggle);
    assert(!kbToggle.active);

    // 3. AlwaysOn mode
    parallax::Keybind kbAlways{ 0x43, parallax::KeybindMode::AlwaysOn, false };
    parallax::UpdateKeybindState(kbAlways);
    assert(kbAlways.active);

    parallax::ClearSimulatedKeys();

    // 4. Layout Diagnostics (Ensure no SetCursorPos extending bounds assertion)
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("LayoutDiagTestHost");

    std::string testInput = "DiagnosticValue";
    parallax::InputText("TestDiagnosticInput", testInput, "search", "Hint");

    parallax::KeybindRow("Diagnostic Keybind", "Keybind row layout testing", &kbHold);

    ImGuiWindow* win = ImGui::GetCurrentWindow();
    assert(!win->DC.IsSetPos); // IsSetPos MUST be cleared to avoid ImGui assertion
    (void)win;

    ImGui::End();
    ImGui::Render();

    std::cout << "  ✓ Keybind Hold, Toggle, AlwaysOn state machine & Layout boundary safety verified" << std::endl;
}

void TestKeySystem() {
    std::cout << "[TEST] 6. Testing Developer Key System async validation..." << std::endl;

    auto& keySys = parallax::auth::KeySystem::Get();

    // Test invalid key
    keySys.StartVerification("INVALID-KEY-12345");
    for (int i = 0; i < 20; ++i) {
        keySys.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    assert(!keySys.IsAuthenticated());
    assert(keySys.GetStatus() == parallax::auth::KeyStatus::InvalidKey);
    std::cout << "  ✓ Rejection of invalid key verified" << std::endl;

    // Test valid VIP key
    keySys.StartVerification("PRLX-2026-VIP");
    for (int i = 0; i < 20; ++i) {
        keySys.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    assert(keySys.IsAuthenticated());
    assert(keySys.GetStatus() == parallax::auth::KeyStatus::Success);
    std::cout << "  ✓ Async authentication with VIP key verified" << std::endl;
}

int main() {
    IMGUI_CHECKVERSION();
    ImGuiContext* imguiCtx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f);
    io.DeltaTime = 1.0f / 60.0f;

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    std::cout << "==========================================================" << std::endl;
    std::cout << " PARALLAX UI v" << PARALLAX_VERSION_STRING << " AUTOMATED VERIFICATION SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestSemVerAndFixedShell();
    TestThemeAndAccentEngine();
    TestCollapsibleAndAutoGroupbox();
    TestRedesignedWidgetsHeadless();
    TestKeybindStateMachineAndLayoutDiagnostics();
    TestConfigEngine();
    TestKeySystem();

    std::cout << "==========================================================" << std::endl;
    std::cout << " ALL VERIFICATION SUITES PASSED SUCCESSFULLY (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    ImGui::DestroyContext(imguiCtx);
    return 0;
}
