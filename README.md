# Parallax

**Version 1.1.0** — Parallax is a C++20 desktop user interface framework built on DirectX 11 and Dear ImGui. It provides a cohesive, token-driven immediate-mode design system featuring a 5-layer surface hierarchy, dual themes (**Graphite Dark** and **Porcelain Light**), restraint-first accent presets, physics-driven animations, an on-demand SVG icon rasterization atlas, automatic layout cards, a JSON configuration manager, and an optional license key verification module.

---

## Overview

While Dear ImGui is traditionally used for developer overlays and debug panels, Parallax transforms it into a structured desktop UI library suitable for client applications, configuration suites, and technical dashboards. Parallax replaces ad-hoc ImGui styling with a centralized design token architecture (`StyleColors` and `StyleMetrics`), custom-rendered interactive widgets, automatic card sizing, and hardware-accelerated vector icon caching.

---

## Key Features in v1.1.0

- **5-Layer Visual Hierarchy & Design Tokens**:
  - Clear visual separation across Application Shell (`WindowBg`), Navigation Sidebar (`HeaderBg`), Workspace Canvas (`ContentBg`), Elevated Content Cards (`CardBg` / `CardHeaderBg`), and Interactive Controls (`WidgetBg` / `WidgetHover` / `WidgetActive`).
- **Dual Theme Engine (`Graphite` Dark & `Porcelain` Light)**:
  - **Graphite (`ThemeMode::Dark`, default)**: Deep graphite layered surfaces, soft neutral borders, and crisp off-white typography.
  - **Porcelain (`ThemeMode::Light`)**: Purpose-built warm off-white palette with elevated white cards, dark graphite typography, and subtle ambient shadows.
  - Smooth, flicker-free animated color interpolation when switching themes at runtime, including Win32 DWM titlebar adaptation.
- **Restraint-First Accent System**:
  - Default **Muted Sage (`#78A18B`)** visual identity alongside **Monochrome**, **Desaturated Copper**, **Subtle Steel Blue**, and **Custom RGBA** presets with automatic contrast-safe token derivation for both Dark and Light modes.
- **Bespoke Parallax Widget Suite**:
  - **Toggles & Checkboxes**: Standard and compact pill toggles (`Toggle`, `ToggleCompact`), animated checkboxes (`Checkbox`), indeterminate checkboxes (`CheckboxIndeterminate`), and chained `ControlRow` builders with descriptions, disabled states, inline keybinds, and color pickers.
  - **Precision Sliders & Dual Range Sliders**: Single-value (`SliderFloat`, `SliderInt`, `SliderFloatEx`) and dual-handle range sliders (`RangeSliderFloat`, `RangeSliderInt`) with machined pill thumbs, center grip notches, optional tick marks, floating drag callouts, and **double-click / Ctrl+Click inline numeric editing**.
  - **Buttons**: `Primary`, `Secondary`, `Subtle` (`Ghost`), and `Destructive` (`Danger`) variants, compact buttons (`ButtonCompact`), square icon buttons (`IconButton`), and extended `ButtonEx` options with animated loading spinners and shortcut badges.
  - **Dropdowns & Multi-Selects**: Single-select (`Dropdown`) and tag-summarized multi-select (`MultiDropdown`) popovers with integrated live search filters and SVG icons.
  - **Color Pickers**: HSV saturation-value square, hue/alpha bars, hex input, curated swatch palette, and full-row `ColorPickerRow` helper.
  - **Text Inputs & Keybinds**: Icon-prefixed text and password inputs (`InputText` with visibility toggle) and interactive key/mouse binding pills (`KeybindWidget` with `Hold` / `Toggle` / `Always On` context menu).
- **Automatic Card Sizing & Fixed Application Shell**:
  - Fixed `1137 × 699` outer menu dimensions with collapsible `220px` / `64px` sidebar navigation.
  - `BeginGroupbox` automatically sizes its height to fit child widgets when `height <= 0.0f`, eliminating manual pixel height estimation and clipped content.
  - `CollapsibleGroupbox` with animated chevron rotation and vertical space reclamation.
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
1. Semantic Versioning (`1.1.0`) constants and `1137 × 699` fixed window dimensions.
2. Theme Engine transitions (`Graphite` Dark ↔ `Porcelain` Light) and Accent Preset token derivation.
3. Auto-sizing `Groupbox` content fitting and `CollapsibleGroupbox` space reclamation.
4. Headless rendering of all redesigned custom widgets across Dark and Light themes.
5. JSON `ConfigManager` atomic profile persistence, Theme/Accent serialization, and Base64 import/export.
6. Asynchronous `KeySystem` rejection and VIP authentication flows.

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
    parallax::SetTheme(parallax::ThemeMode::Dark);          // Graphite Dark (default) or ThemeMode::Light (Porcelain)
    parallax::SetAccentPreset(parallax::AccentPreset::Sage); // Sage (default), Monochrome, Copper, SteelBlue, Custom
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
