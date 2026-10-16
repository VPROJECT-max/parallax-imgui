# Parallax

**Version 1.2.0** — Parallax is a C++20 desktop user interface framework built on DirectX 11 and Dear ImGui. It provides a cohesive, token-driven immediate-mode design system featuring a 5-layer surface hierarchy, three themes (**AMOLED Black**, **Graphite Dark**, and **Porcelain Light**), the canonical **#9998F6** brand accent, smooth synchronized scrolling, global search navigation, physics-driven animations, an on-demand SVG icon rasterization atlas, automatic layout cards, a JSON configuration manager, and an optional license key verification module.

---

## Overview

While Dear ImGui is traditionally used for developer overlays and debug panels, Parallax transforms it into a structured desktop UI library suitable for client applications, configuration suites, and technical dashboards. Parallax replaces ad-hoc ImGui styling with a centralized design token architecture (`StyleColors` and `StyleMetrics`), custom-rendered interactive widgets, automatic card sizing, and hardware-accelerated vector icon caching.

---

## Key Features in v1.2.0

- **5-Layer Visual Hierarchy & Surface Separation**:
  - Clear visual separation across Application Shell (`WindowBg`), Navigation Sidebar (`HeaderBg`), Workspace Canvas (`ContentBg`), Elevated Content Cards (`CardBg` / `CardHeaderBg`), and Interactive Controls (`WidgetBg` / `WidgetHover` / `WidgetActive`).
- **Three-Tier Theme Engine (`AMOLED Black`, `Graphite Dark` & `Porcelain Light`)**:
  - **AMOLED (`ThemeMode::Amoled`, default)**: True `#000000` base with strict 5-layer dark-gray surfaces (`#050507`, `#0A0B0E`, `#0F1015`, `#14161C`, `#1C1F28`) ensuring cards and controls never blend together, paired with high-contrast `#F5F6FA` typography.
  - **Graphite (`ThemeMode::Dark`)**: Deep graphite layered surfaces, soft neutral borders, and readable off-white typography.
  - **Porcelain (`ThemeMode::Light`)**: Purpose-built warm off-white palette with elevated white cards, dark graphite typography, and subtle ambient shadows.
  - 3-way segmented `ThemeSwitcher` toggle with spring-physics active pill and smooth runtime color interpolation.
- **Canonical Brand Accent System (`#9998F6`)**:
  - Default **Parallax Violet (`#9998F6`, `AccentPreset::Parallax`)** alongside **Muted Sage**, **Monochrome**, **Desaturated Copper**, **Subtle Steel Blue**, and **Custom RGBA** presets with automatic contrast-safe token derivation across all themes.
- **Pinned Workspace Header & Global Search Bar**:
  - Decoupled, fixed header bar housing the live section title, search input, quick theme switcher, and resolution metrics.
  - Search catalog indexing all tabs, sections, and controls with case-insensitive token filtering.
  - Interactive search popover with metadata badges; selecting an entry navigates to the tab, expands collapsible sections, and highlights the target card with a pulsing accent spotlight outline.
- **Synchronized Inertial Scrolling Architecture**:
  - Dedicated content scroll region isolating mouse wheel input from Dear ImGui's internal handler, eliminating judder and scroll-fighting.
  - 1-frame height pop on tab switching eliminated via synchronized scroll target resets.
- **Original Parallax Multi-Layer Vector Branding**:
  - Restored authentic `assets/logo.svg` multi-layer emblem in sidebar and navigation bars with un-premultiplied straight RGBA alpha blending.
- **Bespoke Parallax Widget Suite**:
  - **Toggles & Checkboxes**: Standard and compact pill toggles (`Toggle`, `ToggleCompact`), animated checkboxes (`Checkbox`), indeterminate checkboxes (`CheckboxIndeterminate`), and chained `ControlRow` builders.
  - **Precision Sliders & Dual Range Sliders**: Single-value and dual-handle range sliders with machined pill thumbs, floating drag callouts, and **double-click / Ctrl+Click inline numeric editing**.
  - **Interactive Keybinds**: `KeybindWidget` and `KeybindRow` supporting keyboard and mouse buttons (M1–M5), Hold/Toggle/Always-On modes, release gating, and conflict-free hotkey handling.
  - **Buttons**: `Primary`, `Secondary`, `Subtle` (`Ghost`), and `Destructive` variants, compact buttons (`ButtonCompact`), square icon buttons (`IconButton`), and `ButtonEx` with spinners and badges.
  - **Dropdowns & Multi-Selects**: Single-select (`Dropdown`) and tag-summarized multi-select (`MultiDropdown`) with live search filters.
  - **Color Pickers**: HSV canvas, alpha bar, hex input, swatch palette, and `ColorPickerRow`.
  - **Automatic Card Sizing**: Auto-fitting `BeginGroupbox` and animated `CollapsibleGroupbox`.
- **On-Demand SVG Icon Atlas**:
  - Embedded LunaSVG rasterization pipeline packing over 2,077 Lucide vector icons dynamically into DirectX 11 texture memory with per-monitor DPI scaling.
- **JSON Configuration & Key Verification**:
  - Atomic profile persistence (`ConfigManager`) with Base64 import/export and automatic Theme/Accent state serialization.
  - Optional asynchronous license verification modal (`KeySystem`) with DPAPI-encrypted local credential storage.

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

## Building & Verification

```bash
# Clone repository
git clone https://github.com/VPROJECT-max/parallax-imgui.git
cd parallax-imgui

# Configure and build Release targets
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Running the Headless Test Suite

```bash
.\build\Release\test_headless.exe
```

The automated test suite verifies:
1. Semantic Versioning (`1.2.0`) constants and `1137 × 699` fixed window dimensions.
2. Theme Engine transitions (`AMOLED` Black ↔ `Graphite` Dark ↔ `Porcelain` Light) and canonical `#9998F6` Accent derivation.
3. Auto-sizing `Groupbox` content fitting and `CollapsibleGroupbox` space reclamation.
4. Headless rendering of all custom widgets across Dark and Light themes.
5. Keybind Hold, Toggle, Always-On state machines, simulation hooks, and layout boundary safety.
6. JSON `ConfigManager` atomic profile persistence, Theme/Accent serialization, and Base64 import/export.
7. Asynchronous `KeySystem` rejection and VIP authentication flows.

### Launching the DirectX 11 Showcase

```bash
.\build\Release\parallax_demo_dx11.exe
```

---

## Quick Start

```cpp
#include <parallax/parallax.hpp>

void OnDeviceCreated(ID3D11Device* device, ID3D11DeviceContext* context) {
    parallax::Init(device, context);

    // Optional: Configure default theme and accent preset
    parallax::SetTheme(parallax::ThemeMode::Amoled);              // AMOLED Black (default), Dark (Graphite), or Light (Porcelain)
    parallax::SetAccentPreset(parallax::AccentPreset::Parallax);  // Canonical Parallax Violet #9998F6 (default)
}

void RenderUI() {
    static bool menuOpen = true;
    static int activeTab = 0;

    parallax::RenderNotifications();
    parallax::Watermark("PARALLAX v1.1.0", "Active Session", true, true);

    if (parallax::BeginWindow("Parallax Application", &menuOpen)) {
        // Collapsible sidebar navigation
        if (parallax::BeginNavBar()) {
            if (parallax::NavTab("sliders", "Components", activeTab == 0)) activeTab = 0;
            if (parallax::NavTab("palette", "Appearance", activeTab == 1)) activeTab = 1;
            parallax::EndNavBar();
        }

        // Header controls (Accent selector + Dark/Light theme switcher)
        parallax::AccentSelector("HeaderAccent");
        ImGui::SameLine(0.0f, 8.0f);
        parallax::ThemeSwitcher("HeaderTheme");

        // Two-column responsive grid with auto-sizing groupboxes
        parallax::BeginGrid(2);

        if (parallax::BeginGroupbox("Interactive Controls", "sliders")) {
            parallax::GroupboxDescription("Auto-sized card with chained rows and precision sliders.");

            static bool enabled = true;
            static int hotkey = 0;
            parallax::ControlRow("Enable Module", &enabled)
                .Description("Toggle primary feature with an inline keybind")
                .Keybind(&hotkey)
                .Render();

            static float sensitivity = 1.25f;
            parallax::SliderFloat("Sensitivity", &sensitivity, 0.1f, 5.0f, "%.2fx");

            parallax::EndGroupbox();
        }

        parallax::NextColumn();

        if (parallax::BeginGroupbox("Actions", "zap")) {
            if (parallax::Button("Apply Changes", "check", parallax::ButtonVariant::Primary)) {
                parallax::Notify(parallax::NotificationLevel::Success, "Saved", "Configuration applied.");
            }
            parallax::EndGroupbox();
        }

        parallax::EndGrid();
        parallax::EndWindow();
    }
}

void OnDeviceDestroyed() {
    parallax::Shutdown();
}
```

---

## Versioning Policy

Parallax follows **Semantic Versioning (`MAJOR.MINOR.PATCH`)**. See [CHANGELOG.md](CHANGELOG.md) for detailed release notes and version history.

---

## License

All rights reserved. Licensing terms will be specified upon public release.
