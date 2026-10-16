# Getting Started with Parallax UI

Parallax is a high-performance C++20 user-interface framework designed for DirectX 11 and Dear ImGui. It provides a cohesive design system, an AMOLED-optimized 5-layer surface hierarchy, animated navigation, custom interactive widgets, layout presets, JSON configuration persistence, and an optional asynchronous key verification system.

---

## System Requirements

- **Operating System:** Windows 10 / Windows 11 (64-bit)
- **C++ Compiler:** MSVC 19.30+ (Visual Studio 2022) with `/std:c++20`
- **Graphics API:** DirectX 11 (Feature Level 11_0+)
- **Required System Libraries:**
  - `d3d11.lib`
  - `dxgi.lib`
  - `d3dcompiler.lib`
  - `dwmapi.lib`
  - `windowscodecs.lib`

---

## SDK Installation & Structure

Parallax is distributed as a standalone compiled SDK containing precompiled static libraries, public C++20 headers, and runtime assets.

```
sdk/
├── include/
│   ├── parallax/       # Public Parallax C++ headers
│   └── imgui/          # Dear ImGui headers (v1.93.0 docking)
├── lib/
│   ├── parallax.lib    # Unified standalone static library (includes ImGui, LunaSVG, PlutoVG)
│   └── modular/        # Individual libraries (parallax_core.lib, imgui.lib, lunasvg.lib, plutovg.lib)
├── assets/
│   ├── fonts/          # Inter font family (Regular, Medium, SemiBold, Bold)
│   ├── icons/          # 2,077 SVG icons
│   └── logo.svg        # Official Parallax brand SVG mark
└── cmake/
    ├── ParallaxConfig.cmake
    └── ParallaxConfigVersion.cmake
```

---

## CMake Integration

Add the Parallax SDK to your project using standard CMake packages:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_application LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point CMake to your Parallax SDK folder
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sdk")
find_package(Parallax REQUIRED)

add_executable(my_application main.cpp)
target_link_libraries(my_application PRIVATE Parallax::Parallax)

# Copy runtime assets next to your executable
add_custom_command(TARGET my_application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PARALLAX_ASSETS_DIR}/fonts" "$<TARGET_FILE_DIR:my_application>/assets/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PARALLAX_ASSETS_DIR}/icons" "$<TARGET_FILE_DIR:my_application>/icons"
)
```

---

## First Application (Declarative WindowBuilder)

```cpp
#include <parallax/parallax.hpp>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // 1. Initialize DirectX 11 standalone host window
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1280, 800, L"My Parallax Application")) {
        return 1;
    }

    // 2. Initialize Parallax UI Engine
    parallax::Init(host.GetDevice(), host.GetContext());

    // 3. Application State
    bool menuOpen = true;
    bool enableFeature = true;
    float frequency = 440.0f;

    // 4. Construct Window Declaratively
    parallax::WindowBuilder builder("Control Panel");
    builder.WithTheme(parallax::ThemeMode::Amoled)
           .WithAccent(parallax::AccentPreset::Parallax)
           .AddTab("sliders", "Synthesizer", [&](parallax::TabContext& ctx) {
               ctx.BeginGrid(2);
               ctx.Groupbox("Oscillator", [&]() {
                   ctx.Toggle("Enable Generator", &enableFeature);
                   ctx.SliderFloat("Frequency (Hz)", &frequency, 20.0f, 2000.0f, "%.1f Hz");
               }, "activity");
               ctx.EndGrid();
           });

    // 5. Run Render Loop
    host.RunMainLoop([&]() {
        builder.Render(&menuOpen);
    });

    // 6. Clean Shutdown
    parallax::Shutdown();
    host.Shutdown();
    return 0;
}
```

---

## Rendering Lifecycle

1. **Host Setup:** Initialize host window and DirectX 11 swap chain (`D3D11Host::InitStandalone` or `InitExisting`).
2. **Library Init:** Call `parallax::Init(device, context)` once. This loads fonts, initializes the design tokens, and prepares the LunaSVG texture atlas.
3. **Per-Frame Loop:**
   - Update input and keybinds: `parallax::UpdateKeybindState(bind)`.
   - Update configuration auto-save: `parallax::config::ConfigManager::Get().Update(dt)`.
   - Update key verification status: `parallax::auth::KeySystem::Get().Update()`.
   - Render toasts and watermarks: `parallax::RenderNotifications()`, `parallax::Watermark(...)`.
   - Render main window: `builder.Render(&menuOpen)` or `parallax::BeginWindow(...)` / `parallax::EndWindow()`.
4. **Shutdown:** Call `parallax::Shutdown()` before destroying device context or swap chain.
