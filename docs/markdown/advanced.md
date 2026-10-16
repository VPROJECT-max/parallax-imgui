# Advanced Systems & Architecture

---

## JSON Configuration Engine (`ConfigManager`)

Parallax includes an atomic profile configuration manager that binds runtime variables to persistent JSON storage.

```cpp
#include <parallax/config/config_system.hpp>

auto& cfg = parallax::config::ConfigManager::Get();
cfg.SetConfigDirectory("configs");

// Bind variables
cfg.BindBool("aimbot_enabled", &aimbotEnabled, true);
cfg.BindFloat("aimbot_fov", &aimbotFov, 90.0f);
cfg.BindKeybind("aimbot_key", &aimKey);

// Atomic save/load
cfg.SaveProfile("Default");
cfg.LoadProfile("Default");

// Base64 sharing
std::string shareCode = cfg.ExportToBase64();
cfg.ImportFromBase64(shareCode);

// Main loop debounced auto-save update
cfg.Update(ImGui::GetIO().DeltaTime);
```

---

## Developer Key Verification System (`KeySystem`)

An asynchronous authentication module supporting background verification, local license caching, and modal dialogs.

```cpp
#include <parallax/auth/key_system.hpp>

auto& keySys = parallax::auth::KeySystem::Get();

// Set custom verifier lambda
keySys.SetVerifier([](const std::string& key) {
    return key == "PRLX-2026-VIP" || key == "PRLX-DEMO-PRO";
});

// Start non-blocking verification
keySys.StartVerification("PRLX-2026-VIP");

// Main loop update
keySys.Update();

if (keySys.IsAuthenticated()) {
    // Grant access to licensed features
}

// Render authentication modal dialog
bool showModal = true;
keySys.RenderModal(&showModal);
```

---

## Standalone Host vs Game / CAD Hook Mode

The `D3D11Host` supports two operational paradigms:

### Mode A: Standalone Host Window
Spins up a native Win32 window with Direct3D 11 swap chain and message pump:
```cpp
parallax::dx11::D3D11Host host;
host.InitStandalone(hInstance, 1280, 800, L"Standalone Host");
```

### Mode B: Existing Swap Chain Hook
Attaches to an existing Direct3D 11 Present hook (e.g. game overlay, CAD tool, emulator):
```cpp
parallax::dx11::D3D11Host host;
host.InitExisting(device, context, swapChain);

// Inside your IDXGISwapChain::Present hook:
host.BeginFrame();
// Render Parallax UI
host.EndFrame(false);
```
