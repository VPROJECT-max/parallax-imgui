# Parallax

Parallax is a C++20 desktop user interface library built on top of DirectX 11 and Dear ImGui. It provides an immediate-mode UI system featuring custom aesthetics, physics-driven spring animations, an on-demand SVG icon rasterization atlas, structured responsive layouts, a JSON configuration manager, and an optional license key verification module.

---

## Overview

While Dear ImGui is traditionally utilized for debugging consoles and developer tooling, Parallax provides a cohesive, polished desktop interface framework suitable for client applications, configuration utilities, and overlay dashboards. Parallax abstracts lower-level ImGui cursor math and manual drawing, offering structured layout grids, chained widget rows, typography management, and hardware-accelerated icon caching.

---

## Features

- **Custom Immediate-Mode Aesthetic**: Multi-layered dark themes, high-contrast borders, consistent padding, and typography hierarchy powered by the Inter font family.
- **On-Demand SVG Icon Atlas**: Embedded pipeline backed by LunaSVG capable of rasterizing over 2,000 vector icons on demand directly into dynamic DirectX 11 texture memory with per-monitor DPI scaling.
- **Physics-Driven Motion**: Damped harmonic spring physics and easing curves for interactive widget hover states, modal popups, and sidebar transitions.
- **Custom Widget Suite**:
  - Checkboxes and animated pill switches.
  - Chained control rows with inline sub-widgets (keybinds, color pickers).
  - Single-select and multi-select dropdowns with integrated search and icons.
  - Floating-point and integer sliders, single and two-line formats, and dual-thumb range sliders.
  - Collapsible cards and groupboxes with animated disclosure chevrons and space reclamation.
  - Text and password inputs with leading icon indicators.
  - Stacked transient toast notifications with severity levels.
  - Fixed-dimension application shells (1137 × 699) with collapsible icon/expanded sidebars.
- **JSON Configuration Engine**: Variable binding system with atomic disk writes, profile switching, and compact Base64 configuration import/export.
- **Modular Key Verification System**: Optional cryptographic and remote authorization module with asynchronous status polling, persistent encrypted credential storage, and non-blocking dialogs.

---

## Requirements

- **Operating System**: Windows 10 / Windows 11 (64-bit)
- **Graphics API**: DirectX 11 (Direct3D 11 runtime, DXGI, D3DCompiler, DWM)
- **Compiler**: C++20 compliant toolset (MSVC v143 / Visual Studio 2022 recommended, or Clang-cl)
- **Build System**: CMake 3.20 or newer

All third-party dependencies are fetched automatically during CMake configuration via `FetchContent`:
- [Dear ImGui](https://github.com/ocornut/imgui) (Docking branch v1.93.0)
- [LunaSVG](https://github.com/sammycage/lunasvg) (v3.5.0 with PlutoVG)
- [nlohmann/json](https://github.com/nlohmann/json) (v3.11.3)

---

## Building

Clone the repository and build using standard CMake commands:

```bash
# Clone repository
git clone https://github.com/VPROJECT-max/parallax-imgui.git
cd parallax-imgui

# Configure build directory
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Compile library, demo, and test suite
cmake --build build --config Release
```

### Running Tests

Execute the automated headless verification suite:

```bash
.\build\Release\test_headless.exe
```

The test runner validates:
1. Collapsible groupbox layout expansion and space reclamation.
2. JSON configuration manager serialization, atomic file writes, and Base64 export/import.
3. Asynchronous key system verification and failure rejection.

### Launching the Demonstration

```bash
.\build\Release\parallax_demo_dx11.exe
```

---

## Quick Start

The following snippet demonstrates basic initialization and rendering of a Parallax interface within an existing DirectX 11 ImGui application:

```cpp
#include <parallax/parallax.hpp>

// 1. Initialize Parallax with your existing DirectX 11 device and context
void OnDeviceCreated(ID3D11Device* device, ID3D11DeviceContext* context) {
    parallax::Init(device, context);
}

// 2. Render interface during your application's frame loop
void RenderUI() {
    static bool menuOpen = true;
    static int activeTab = 0;

    // Optional: Render floating toast notifications and watermark
    parallax::RenderNotifications();
    parallax::Watermark("APPLICATION v1.0", "Active Session", true, true);

    // Render fixed-dimension application window
    if (parallax::BeginWindow("Parallax Dashboard", &menuOpen)) {
        // Collapsible navigation sidebar
        if (parallax::BeginNavBar()) {
            if (parallax::NavTab("shield", "Security", activeTab == 0)) activeTab = 0;
            if (parallax::NavTab("sliders", "Settings", activeTab == 1)) activeTab = 1;
            parallax::EndNavBar();
        }

        // Two-column responsive layout
        parallax::BeginGrid(2);

        if (parallax::BeginGroupbox("Protection Settings", "shield")) {
            static bool realTimeProtection = true;
            parallax::Checkbox("Real-time Shield", &realTimeProtection);

            static float sensitivity = 1.0f;
            parallax::SliderFloat("Scan Sensitivity", &sensitivity, 0.1f, 5.0f);

            parallax::EndGroupbox();
        }

        parallax::NextColumn();

        if (parallax::BeginGroupbox("Configuration", "sliders")) {
            static std::string apiKey = "sk-live-default";
            parallax::InputText("License Key", apiKey, "key", "Enter key...", 0.0f, true);

            if (parallax::Button("Save Configuration", "check", parallax::ButtonVariant::Primary)) {
                parallax::Notify(parallax::NotificationLevel::Success, "Saved", "Settings updated.");
            }

            parallax::EndGroupbox();
        }

        parallax::EndGrid();

        parallax::EndWindow();
    }
}

// 3. Clean up on shutdown
void OnDeviceDestroyed() {
    parallax::Shutdown();
}
```

---

## Architecture

The library is organized into self-contained modules:

- **Core (`parallax::core`)**: Global context management, high-DPI scaling computations, style metrics, and typography loaders.
- **Rendering Backend (`parallax::dx11`)**: Standalone Win32/D3D11 host implementation with dark-mode window framing and swap chain presentation.
- **Vector Icon Pipeline (`parallax::icons`)**: Dynamic texture packing atlas that resolves Lucide SVG icons from disk, rasterizes them to target pixel dimensions via LunaSVG, and uploads sub-rectangles to D3D11 textures.
- **Animation System (`parallax::anim`)**: Spring simulators and easing functions driving smooth numeric transitions across frame deltas.
- **Layout Manager (`parallax::layout`)**: Coordinate tracking for sidebars, navigation tabs, grid reflows, and groupbox containers.
- **Widget Implementations (`parallax::widgets`)**: Immediate-mode control definitions handling pointer interactions, state caches, and rendering primitives.
- **Configuration Engine (`parallax::config`)**: Type-erased variable binding table serializing state to JSON via atomic temporary file swapping.
- **Authorization Module (`parallax::auth`)**: Asynchronous license validation modal with support for custom verification callbacks.

---

## Project Structure

```text
parallax-imgui/
├── assets/                  # Inter font family definitions and branding SVGs
│   ├── fonts/               # Static Inter font weights (.ttf)
│   └── logo.svg             # Application vector logo
├── configs/                 # Runtime directory for saved JSON profiles
├── docs/                    # Technical specifications and developer notes
├── examples/
│   └── demo_dx11/           # Standalone DirectX 11 reference application
├── icons/                   # 2,077 Lucide SVG vector icons
├── include/parallax/        # Public API headers
│   ├── anim/                # Spring physics and easing engines
│   ├── auth/                # Optional Key authorization system
│   ├── config/              # JSON configuration engine
│   ├── core/                # Context, style definitions, and typography
│   ├── dx11/                # DirectX 11 host and rendering abstractions
│   ├── icons/               # SVG rasterization and dynamic atlas
│   ├── input/               # Global hotkey dispatching
│   ├── layout/              # Windows, navbars, sub-tabs, and grids
│   └── widgets/             # Checkboxes, sliders, dropdowns, inputs, buttons
├── src/                     # Implementation files
├── tests/                   # Headless automated verification suite
├── CMakeLists.txt           # Build configuration script
└── README.md
```

---

## Current Status

Parallax is currently in a stable development checkpoint. Core layout math, icon alignment, component state management, and configuration persistence have been audited and verified through automated test suites. Future development will focus on extended documentation, additional backend targets, and expanded widget primitives.

---

## License

All rights reserved. Licensing terms will be specified upon public release.
