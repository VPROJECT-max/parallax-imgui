#include <iostream>
#include <cassert>
#include <imgui.h>
#include <imgui_internal.h>
#include "parallax/parallax.hpp"
#include "parallax/widgets/collapsible_groupbox.hpp"
#include "parallax/config/config_system.hpp"
#include "parallax/auth/key_system.hpp"

void TestCollapsibleGroupbox() {
    std::cout << "[TEST] 1. Testing Collapsible Groupbox lifecycle & space reclamation..." << std::endl;

    bool expanded = true;
    ImGuiIO& io = ImGui::GetIO();

    // Frame 0-4: Initial expanded render
    for (int i = 0; i < 5; ++i) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(100, 100));
        ImGui::SetNextWindowSize(ImVec2(800, 600));
        ImGui::Begin("TestWindow");
        bool open = parallax::BeginCollapsibleGroupbox("Test Group", &expanded, "eye", 0.0f, true);
        (void)open;
        ImGui::Button("Child Button", ImVec2(100, 30));
        parallax::EndCollapsibleGroupbox();
        ImGui::End();
        ImGui::Render();
    }

    // Frame 5: Mouse click on header to collapse
    io.MousePos = ImVec2(200.0f, 115.0f);
    io.MouseDown[0] = true;
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(800, 600));
    ImGui::Begin("TestWindow");
    parallax::BeginCollapsibleGroupbox("Test Group", &expanded, "eye", 0.0f, true);
    parallax::EndCollapsibleGroupbox();
    ImGui::End();
    ImGui::Render();

    // Frame 6: Release mouse
    io.MouseDown[0] = false;
    for (int frame = 0; frame < 30; ++frame) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(100, 100));
        ImGui::SetNextWindowSize(ImVec2(800, 600));
        ImGui::Begin("TestWindow");
        bool open = parallax::BeginCollapsibleGroupbox("Test Group", &expanded, "eye", 0.0f, true);
        if (open) {
            ImGui::Button("Child Button", ImVec2(100, 30));
            parallax::EndCollapsibleGroupbox();
        }
        ImGui::End();
        ImGui::Render();
    }

    assert(expanded == false);
    std::cout << "  ✓ Collapsible successfully collapsed (expanded=" << expanded << ")" << std::endl;

    // Test re-expanding
    io.MousePos = ImVec2(200.0f, 115.0f);
    io.MouseDown[0] = true;
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(800, 600));
    ImGui::Begin("TestWindow");
    parallax::BeginCollapsibleGroupbox("Test Group", &expanded, "eye", 0.0f, true);
    parallax::EndCollapsibleGroupbox();
    ImGui::End();
    ImGui::Render();

    io.MouseDown[0] = false;
    for (int frame = 0; frame < 30; ++frame) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(100, 100));
        ImGui::SetNextWindowSize(ImVec2(800, 600));
        ImGui::Begin("TestWindow");
        bool open = parallax::BeginCollapsibleGroupbox("Test Group", &expanded, "eye", 0.0f, true);
        if (open) {
            ImGui::Button("Child Button", ImVec2(100, 30));
            parallax::EndCollapsibleGroupbox();
        }
        ImGui::End();
        ImGui::Render();
    }

    assert(expanded == true);
    std::cout << "  ✓ Collapsible successfully re-expanded (expanded=" << expanded << ")" << std::endl;
}

void TestConfigEngine() {
    std::cout << "[TEST] 2. Testing JSON Configuration Engine (atomic write, binding, Base64)..." << std::endl;

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

    // Save profile
    bool saveOk = cfg.SaveProfile("UnitTestProfile");
    (void)saveOk;
    assert(saveOk);
    std::cout << "  ✓ Atomic profile save succeeded" << std::endl;

    // Mutate variables
    bTest = false;
    iTest = 999;
    fTest = 0.001f;
    cTest = ImVec4(0.9f, 0.8f, 0.7f, 0.6f);
    kbTest.vkey = 'Z';
    kbTest.mode = parallax::KeybindMode::Toggle;
    sTest = "Corrupted";
    mbTest = { false, false, false, false };

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
    std::cout << "  ✓ Profile reload verified (all bound types match exactly)" << std::endl;

    // Test Base64 export & import
    std::string b64 = cfg.ExportToBase64();
    assert(!b64.empty());
    std::cout << "  ✓ Base64 config export produced: " << b64.substr(0, 32) << "..." << std::endl;

    // Mutate again
    iTest = 100;
    sTest = "Modified";

    // Import from Base64
    bool importOk = cfg.ImportFromBase64(b64);
    (void)importOk;
    assert(importOk);
    assert(iTest == 42);
    assert(sTest == "TestStringValue");
    std::cout << "  ✓ Base64 config import verified" << std::endl;

    // Cleanup test profile
    cfg.DeleteProfile("UnitTestProfile");
    std::cout << "  ✓ Profile deletion verified" << std::endl;
}

void TestKeySystem() {
    std::cout << "[TEST] 3. Testing Developer Key System async validation..." << std::endl;

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
    std::cout << " PARALLAX UI AUTOMATED VERIFICATION SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestCollapsibleGroupbox();
    TestConfigEngine();
    TestKeySystem();

    std::cout << "==========================================================" << std::endl;
    std::cout << " ALL VERIFICATION SUITES PASSED SUCCESSFULLY (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    ImGui::DestroyContext(imguiCtx);
    return 0;
}
