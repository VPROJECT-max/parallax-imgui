#!/usr/bin/env python3
"""
Parallax UI Documentation Generator
Builds the static documentation website, Markdown documentation suite,
llms.txt, llms-full.txt, and the 2,078 SVG icon catalog index.
"""

import os
import sys
import json
import shutil

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DOCS_DIR = os.path.join(ROOT_DIR, "docs")
MARKDOWN_DIR = os.path.join(DOCS_DIR, "markdown")
ASSETS_DIR = os.path.join(DOCS_DIR, "assets")

def ensure_dirs():
    os.makedirs(DOCS_DIR, exist_ok=True)
    os.makedirs(MARKDOWN_DIR, exist_ok=True)
    os.makedirs(ASSETS_DIR, exist_ok=True)

def generate_icons_catalog():
    print("[1/5] Indexing 2,078 SVG icons...")
    icons_src = os.path.join(ROOT_DIR, "icons")
    icon_files = [f for f in os.listdir(icons_src) if f.endswith(".svg")]
    icon_slugs = sorted([os.path.splitext(f)[0] for f in icon_files])

    # Categorize common icon prefixes
    categories = {
        "UI & Controls": ["chevron-", "arrow-", "menu", "x", "check", "sliders", "settings", "search", "maximize", "minimize", "plus", "minus", "filter", "more-", "list", "grid"],
        "Hardware & System": ["cpu", "hard-drive", "server", "database", "terminal", "radio", "wifi", "bluetooth", "activity", "zap", "power", "battery", "monitor", "smartphone", "speaker", "volume"],
        "Security & Auth": ["shield", "lock", "unlock", "key", "fingerprint", "eye", "eye-off", "alert-", "help-", "info"],
        "Media & Design": ["image", "palette", "film", "music", "camera", "play", "pause", "square", "circle", "layers", "crop", "brush", "type", "feather"],
        "Communication": ["mail", "message-", "phone", "send", "share", "bell", "inbox", "at-sign", "globe", "users", "user"]
    }

    catalog = []
    for slug in icon_slugs:
        cat = "General"
        for cname, prefixes in categories.items():
            if any(slug.startswith(p) or slug == p.rstrip("-") for p in prefixes):
                cat = cname
                break
        catalog.append({"slug": slug, "category": cat})

    json_path = os.path.join(ASSETS_DIR, "icons_catalog.json")
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(catalog, f, indent=2)
    print(f"  [OK] Indexed {len(catalog)} icons to {json_path}")
    return catalog

def generate_markdown_docs():
    print("[2/5] Generating structured Markdown API documentation...")

    docs = {}

    # 1. Getting Started
    docs["getting-started.md"] = """# Getting Started with Parallax UI

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
"""

    # 2. Core API
    docs["core-api.md"] = """# Core Architecture & Lifecycle API

The Core API manages library initialization, DPI scaling, the fixed 1137x699 window shell, navigation bars, sub-tabs, and layout grids.

---

## Header Inclusion

```cpp
#include <parallax/parallax.hpp>
```

---

## Lifecycle Functions

### `parallax::Init`
Initializes the Parallax UI framework, typography, design token style engine, and on-demand SVG icon atlas.
```cpp
void Init(ID3D11Device* device, ID3D11DeviceContext* context);
```
- **Parameters:**
  - `device`: Valid Direct3D 11 device pointer.
  - `context`: Immediate device context pointer.
- **Preconditions:** Dear ImGui context must already be created (`ImGui::CreateContext()`) and ImGui DX11 backend initialized.
- **Lifetime:** Must be called once during startup.

### `parallax::Shutdown`
Releases all DirectX 11 textures, font objects, cached icon atlas allocations, and animation manager resources.
```cpp
void Shutdown();
```
- **Preconditions:** Call before releasing the Direct3D 11 device and destroying the ImGui context.

---

## DPI Scaling

### `parallax::SetDpiScale` / `parallax::GetDpiScale`
```cpp
void SetDpiScale(float scale);
float GetDpiScale();
```
- Standard baseline scale is `1.0f` (96 DPI). For high-DPI displays (e.g. 144 DPI = `1.5f`, 192 DPI = `2.0f`), set the scale prior to or during execution.

---

## Window Shell Architecture

### Fixed 1137x699 Logical Dimensions
Parallax enforces a strict, calibrated design canvas of **1137 × 699 logical pixels** (scaled by DPI). This guarantees that complex controls, multi-column grids, profile cards, and navigation sidebars maintain mathematically balanced visual rhythm without arbitrary clipping.

### `parallax::BeginWindow` / `parallax::EndWindow`
```cpp
bool BeginWindow(const char* title, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
void EndWindow();
```
- **Parameters:**
  - `title`: Window identifier and caption.
  - `p_open`: Pointer to visibility boolean. Supports smooth spring-animated open and close scaling transitions.
  - `flags`: Optional additional ImGui window flags.
- **Return Value:** Returns `true` if the window content is visible and should be submitted. Always call `EndWindow()` if `BeginWindow()` returns `true`.

---

## Navigation & Sidebars

### `parallax::BeginNavBar` / `parallax::EndNavBar`
```cpp
bool BeginNavBar();
void EndNavBar();
```
- Renders the primary navigation surface. Supports collapsed icon-only mode and expanded mode with spring-damper physics.

### `parallax::NavTab`
```cpp
bool NavTab(const char* icon, const char* label, bool selected, const char* badge = nullptr);
```
- **Parameters:**
  - `icon`: SVG icon slug from the 2,078-icon catalog (e.g. `"sliders"`, `"shield"`, `"palette"`).
  - `label`: Human-readable tab label.
  - `selected`: `true` if this tab is currently active.
  - `badge`: Optional badge text (e.g. `"NEW"`, `"v1.2"`, `"9+"`).
- **Return Value:** Returns `true` if clicked.

### Navigation Position & State Controls
```cpp
void SetNavSide(NavSide side);       // NavSide::Left, NavSide::Right, NavSide::Top, NavSide::Bottom
NavSide GetNavSide();
bool IsNavExpanded();
void SetNavExpanded(bool expanded);
void ToggleNavExpanded();
float GetNavWidth();
```

---

## Sub-Tabs

### `parallax::BeginSubTabs` / `parallax::EndSubTabs` / `parallax::SubTab`
```cpp
bool BeginSubTabs(SubTabPlacement placement = SubTabPlacement::TopHorizontal);
void EndSubTabs();
bool SubTab(const char* label, bool selected);
```
- Renders secondary segmented pill navigation within a workspace tab.

---

## Layout Grid & Groupboxes

### `parallax::BeginGrid` / `parallax::NextColumn` / `parallax::EndGrid`
```cpp
void BeginGrid(int columns = 2, float spacing = 12.0f);
void NextColumn();
void EndGrid();
```
- Multi-column proportional grid system. Balances card widths across available workspace width.

### `parallax::BeginGroupbox` / `parallax::EndGroupbox`
```cpp
bool BeginGroupbox(const char* title, const char* icon = nullptr, float height = 0.0f);
void GroupboxDescription(const char* text);
void EndGroupbox();
```
- Creates an auto-sizing or fixed-height card surface. If `height == 0.0f`, the groupbox automatically sizes itself to fit its content with zero dead chrome.
"""

    # 3. Widgets
    docs["widgets.md"] = """# Widget Catalog & Component API

Parallax includes a comprehensive collection of custom-designed, animated immediate-mode widgets and fluent declarative chaining builders.

---

## Buttons & Interactive Actions

### `parallax::Button`
```cpp
bool Button(const char* label, const char* icon = nullptr, ButtonVariant variant = ButtonVariant::Secondary, const ImVec2& size = ImVec2(0.0f, 0.0f));
```
- **Variants:**
  - `ButtonVariant::Primary`: Filled with primary brand accent color. High emphasis.
  - `ButtonVariant::Secondary`: Outlined widget surface with subtle border. Standard action.
  - `ButtonVariant::Ghost`: Transparent resting state, highlights on hover.
  - `ButtonVariant::Destructive`: Danger red styling for irreversible operations.
- **Example:**
```cpp
if (parallax::Button("Save Configuration", "check", parallax::ButtonVariant::Primary)) {
    // Save logic
}
```

### `parallax::ButtonCompact` / `parallax::IconButton`
```cpp
bool ButtonCompact(const char* label, const char* icon = nullptr, ButtonVariant variant = ButtonVariant::Secondary);
bool IconButton(const char* id, const char* icon, ButtonVariant variant = ButtonVariant::Ghost, float dimension = 30.0f, const char* tooltip = nullptr);
```

---

## Toggles & Checkboxes (Fluent `ControlRow`)

The `ControlRow` API enables chained configuration of switches, checkboxes, descriptions, tooltips, and embedded secondary controls.

```cpp
parallax::Row("Enable Feature", &featureBool)
    .Style(parallax::ToggleStyle::PillSwitch) // or ToggleStyle::Checkbox
    .Description("Detailed explanation of this feature")
    .Tooltip("Quick tooltip guidance")
    .Disabled(false)
    .Compact(false)
    .OnChange([](bool newState) {
        // Callback executed on toggle
    });
```

---

## Sliders & Range Sliders

### `parallax::SliderFloat` / `parallax::SliderInt`
```cpp
bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", float width = 0.0f, bool twoLine = false);
bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", float width = 0.0f, bool twoLine = false);
```
- Features tactile spring-damper thumb physics, animated track filling, and direct numeric input on `Ctrl+Click`.

### `parallax::RangeSliderFloat` / `parallax::RangeSliderInt`
```cpp
bool RangeSliderFloat(const char* label, float* v_min, float* v_max, float min_limit, float max_limit, const char* format = "%.2f", float width = 0.0f);
bool RangeSliderInt(const char* label, int* v_min, int* v_max, int min_limit, int max_limit, const char* format = "%d", float width = 0.0f);
```
- Dual-thumb range selector with automatic collision prevention (`*v_min <= *v_max`).

---

## Dropdowns & Multi-Select

### `parallax::Dropdown`
```cpp
bool Dropdown(const char* label, const std::vector<std::string>& items, int* selected_index, const std::vector<std::string>& item_icons = {}, float width = 0.0f);
```
- Single-selection dropdown with optional SVG icon support for each entry.

### `parallax::MultiDropdown`
```cpp
bool MultiDropdown(const char* label, const std::vector<std::string>& items, std::vector<bool>& selected_states, const std::vector<std::string>& item_icons = {}, float width = 0.0f);
```
- Multi-selection dropdown displaying summary badges (e.g. `"All"`, `"None"`, `"2 Selected"`).

---

## Color Pickers

### `parallax::ColorPicker` / `parallax::ColorPickerRow`
```cpp
bool ColorPicker(const char* label, ImVec4* color, bool hasAlpha = true);
bool ColorPickerRow(const char* label, ImVec4* color, bool hasAlpha = true);
```
- Integrated inline color swatch with popup editor supporting RGB, HSV, Hex, and alpha transparency channels.

---

## Text Inputs & Password Masking

### `parallax::InputText`
```cpp
bool InputText(const char* label, std::string& text, const char* icon = nullptr, const char* placeholder = nullptr, float width = 0.0f, bool isPassword = false);
```
- Clean single-line text input with leading SVG icon, placeholder text, clear button, and password masking toggle.

---

## Keybinds & Mouse Binding

### `parallax::Keybind` Struct
```cpp
struct Keybind {
    int vkey{0};
    KeybindMode mode{KeybindMode::Hold}; // Hold, Toggle, AlwaysOn
    bool active{false};
};
```

### `parallax::KeybindWidget` / `parallax::KeybindRow`
```cpp
bool KeybindWidget(const char* label, parallax::Keybind* keybind);
bool KeybindRow(const char* label, const char* description, parallax::Keybind* keybind);
```
- Supports full keyboard capture, modifier combinations, and mouse buttons (`M1` through `M5`).
- Click-release gating prevents accidental re-binding on mouse assignment.

### `parallax::UpdateKeybindState`
Call once per frame in your main loop to advance hold/toggle state:
```cpp
parallax::UpdateKeybindState(myKeybind);
```

---

## Collapsible Groupboxes

```cpp
bool BeginCollapsibleGroupbox(const char* title, bool* p_expanded, const char* icon = nullptr, float defaultHeight = 0.0f, bool centered = true);
void EndCollapsibleGroupbox();
```
- Animated expandable card. When collapsed, space is reclaimed immediately for smooth layout transitions.

---

## Notification Toasts & Watermarks

### `parallax::Notify`
```cpp
void Notify(NotificationLevel level, const std::string& title, const std::string& message, float duration = 4.0f);
```
- Levels: `NotificationLevel::Success`, `NotificationLevel::Info`, `NotificationLevel::Warning`, `NotificationLevel::Danger`.

### `parallax::RenderNotifications`
Call once at the end of your frame to draw active toasts with spring slide-in transitions:
```cpp
parallax::RenderNotifications();
```

### `parallax::Watermark`
```cpp
void Watermark(const char* title, const char* user = nullptr, bool showFps = true, bool showTime = true);
```
- Floating status pill pinned to the viewport corner displaying framerate and system clock.
"""

    # 4. Appearance & Theming
    docs["appearance.md"] = """# Appearance, Themes & Design System

Parallax features a strict 5-layer surface hierarchy, true `#000000` AMOLED support, the canonical `#9998F6` brand accent, and spring physics.

---

## Theme Engine (3 Presets)

### Theme Modes
```cpp
enum class ThemeMode : uint8_t {
    Amoled = 0, // Pure Black AMOLED (#000000 - Default)
    Dark,       // Graphite Dark
    Light       // Porcelain Light
};
```

### Functions
```cpp
void SetTheme(ThemeMode mode, bool animate = true);
ThemeMode GetTheme() noexcept;
void ToggleTheme(bool animate = true);
bool IsLightTheme() noexcept;
```

---

## 5-Layer Surface Hierarchy (AMOLED Black)

To ensure interface components never blend into background canvases, Parallax employs a mathematically calibrated 5-layer luminance hierarchy:

| Surface Layer | AMOLED Color | Purpose |
|---|---|---|
| **Layer 1: Base Window** | `#000000` | Outer window background canvas |
| **Layer 2: Header Surface** | `#050507` | Pinned top workspace navigation bar |
| **Layer 3: Content Canvas** | `#0A0B0E` | Main tab scrolling workspace |
| **Layer 4: Card Surface** | `#0F1015` | Groupbox & collapsible card backgrounds |
| **Layer 5: Inner Controls** | `#14161C` | Input fields, dropdowns, slider tracks |
| **Interactive Hover** | `#1C1F28` | Hovered and active control surfaces |

---

## Accent Color Engine

### Canonical Primary Brand Accent
- **Hex:** `#9998F6`
- **RGB:** `153, 152, 246`
- **ImVec4:** `(153.0f / 255.0f, 152.0f / 255.0f, 246.0f / 255.0f, 1.0f)`

### Accent Presets
```cpp
enum class AccentPreset : uint8_t {
    Parallax = 0,   // Exact Primary Accent (#9998F6) - Default
    Sage,           // Muted Sage (#78A18B)
    Monochrome,     // Monochrome / Graphite
    Copper,         // Desaturated Copper (#C48A6A)
    SteelBlue,      // Steel Blue (#6E91B0)
    Custom          // User-defined RGB color
};

void SetAccentPreset(AccentPreset preset, bool animate = true);
AccentPreset GetAccentPreset() noexcept;
void SetCustomAccent(const ImVec4& color, bool animate = true);
ImVec4 GetCustomAccent() noexcept;
```

---

## Typography & Inter Fonts

Parallax automatically resolves and loads the official **Inter** font family across 4 weights:
- Regular (`14.5px`)
- Medium (`15.0px`)
- SemiBold (`16.5px`)
- Bold (`18.5px`)
- Title (`22.0px`)

```cpp
parallax::PushFont(parallax::FontWeight::SemiBold);
ImGui::Text("Highlighted Heading");
parallax::PopFont();
```

---

## Animation Engine & Spring Physics

Parallax provides built-in easing and damped harmonic spring solvers:

```cpp
// Easing interpolation
float val = parallax::AnimFloat(ImGuiID id, float target, float duration = 0.25f, anim::EasingPreset preset = anim::EasingPreset::EaseOutQuint);

// Physical spring simulation
float springVal = parallax::SpringFloat(ImGuiID id, float target, const anim::SpringConfig& config = {100.0f, 20.0f, 1.0f});
```
"""

    # 5. Advanced
    docs["advanced.md"] = """# Advanced Systems & Architecture

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
"""

    # 6. Icons
    docs["icons.md"] = """# SVG Icon System & Texture Atlas

Parallax includes a high-performance, on-demand rasterized SVG icon subsystem backed by LunaSVG and Direct3D 11 shelf packing.

---

## Key Features

- **2,078 Embedded Icons:** Full Lucide SVG catalog.
- **On-Demand Rasterization:** Icons are rasterized to high-DPI bitmaps on first reference and packed into a dynamic 2D shelf atlas texture. Unused icons consume zero GPU memory.
- **Theme Tinting:** Monochromatic SVG icons adapt dynamically to active text, accent, or muted colors.
- **Multi-Color Brand Logos:** Support for `preserveColors = true` to render full-fidelity multi-color SVGs (such as the official Parallax brand logo).
- **High-DPI Super-Sampling:** Automatically supersampled up to 256px resolution for crisp rendering on 4K displays.

---

## Drawing Icons

### High-Level Immediate Icon
```cpp
parallax::Icon("search", ImVec2(20.0f, 20.0f), parallax::Colors().Accent);
```

### Direct ImDrawList Drawing
```cpp
ImDrawList* drawList = ImGui::GetWindowDrawList();
parallax::Context::Get().Icons().DrawIcon(
    drawList,
    "sliders",
    screenPos,
    ImVec2(18.0f, 18.0f),
    parallax::Colors().TextPrimary
);
```

---

## Icon Identifiers

Icons are referenced purely by their string slug (e.g. `"sliders"`, `"shield"`, `"activity"`, `"check"`). No file paths or extensions are required. Use the interactive Icon Gallery in this documentation to search, preview, and copy icon slugs.
"""

    # 7. Reference
    docs["reference.md"] = """# Complete Public API Reference

---

## Public Header Map

| Header | Description |
|---|---|
| `<parallax/parallax.hpp>` | Primary entry point; includes all core systems, widgets, and builders |
| `<parallax/core/types.hpp>` | Version constants, `ThemeMode`, `AccentPreset`, `UserProfile`, `NavSide` |
| `<parallax/core/style.hpp>` | `StyleColors`, `StyleMetrics`, and `StyleManager` design tokens |
| `<parallax/core/typography.hpp>` | Font weights, Inter font family management |
| `<parallax/layout/window.hpp>` | `WindowShell` fixed 1137x699 container |
| `<parallax/layout/navbar.hpp>` | Primary navigation bar & tab indicators |
| `<parallax/layout/grid.hpp>` | Proportional multi-column grid and groupbox cards |
| `<parallax/widgets/button.hpp>` | Standard, compact, and icon button controls |
| `<parallax/widgets/chain.hpp>` | `ControlRow` fluent toggle/checkbox chaining builder |
| `<parallax/widgets/slider.hpp>` | Single and dual-range scalar sliders |
| `<parallax/widgets/dropdown.hpp>` | Single and multi-select popup dropdowns |
| `<parallax/widgets/keybind.hpp>` | Keyboard & mouse button binding engine |
| `<parallax/widgets/notification.hpp>` | Toast notification system |
| `<parallax/dx11/d3d11_host.hpp>` | Direct3D 11 host window and swap chain wrapper |
| `<parallax/config/config_system.hpp>` | JSON configuration profile engine |
| `<parallax/auth/key_system.hpp>` | Asynchronous developer license verification |

---

## Version Constants

```cpp
#define PARALLAX_VERSION_MAJOR 1
#define PARALLAX_VERSION_MINOR 2
#define PARALLAX_VERSION_PATCH 0
#define PARALLAX_VERSION_STRING "1.2.0"
#define PARALLAX_VERSION_WIDE L"1.2.0"
```
"""

    # 8. AI Agent Integration Guide
    docs["ai-agent-guide.md"] = """# AI Coding Assistant Integration Guide

This guide provides precise, rule-based instructions for AI coding assistants implementing interfaces using Parallax UI.

---

## Golden Rules for AI Agents

1. **Fixed Window Shell:** Never attempt to resize the main window beyond **1137 × 699**. Parallax automatically enforces this canvas size.
2. **Layout Boundaries:** Do not call raw `ImGui::SetCursorPos()` or `SetCursorScreenPos()` to extend window bounds. Use standard layout builders: `BeginGroupbox()`, `BeginGrid()`, `NextColumn()`, and `Dummy()`.
3. **Scrolling Isolation:** If creating scrollable child windows, always specify `ImGuiWindowFlags_NoScrollWithMouse` and synchronize scrolling with `ImGui::SetNextWindowScroll()`.
4. **Theme Surface Hierarchy:** Always respect the 5-layer surface tokens. Use `parallax::Colors().ContentBg` for workspaces, `CardBg` for cards, and `WidgetBg` for interactive controls.
5. **No Private Headers:** Never `#include` files from `src/` or depend on LunaSVG or nlohmann_json directly. Everything required is exposed cleanly in `<parallax/parallax.hpp>`.

---

## Copy-Paste Integration Boilerplate

```cpp
#include <parallax/parallax.hpp>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1280, 800, L"Application")) return 1;

    parallax::Init(host.GetDevice(), host.GetContext());

    bool open = true;
    bool toggle = true;
    float slider = 50.0f;

    parallax::WindowBuilder builder("Control Panel");
    builder.WithTheme(parallax::ThemeMode::Amoled)
           .WithAccent(parallax::AccentPreset::Parallax)
           .AddTab("sliders", "Settings", [&](parallax::TabContext& ctx) {
               ctx.BeginGrid(2);
               ctx.Groupbox("Configuration", [&]() {
                   ctx.Toggle("Feature Active", &toggle);
                   ctx.SliderFloat("Threshold", &slider, 0.0f, 100.0f);
               }, "settings");
               ctx.EndGrid();
           });

    host.RunMainLoop([&]() {
        builder.Render(&open);
    });

    parallax::Shutdown();
    host.Shutdown();
    return 0;
}
```
"""

    for filename, content in docs.items():
        filepath = os.path.join(MARKDOWN_DIR, filename)
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(content.strip() + "\n")
    print(f"  [OK] Written {len(docs)} Markdown documentation files to {MARKDOWN_DIR}")
    return docs

def generate_llms_files(docs):
    print("[3/5] Generating llms.txt & llms-full.txt...")

    # llms.txt
    llms_txt = """# Parallax UI — Public API Documentation for AI Agents
> High-performance C++20, DirectX 11, Dear ImGui UI framework and design system (v1.2.0).

## Core Capabilities
- Fixed 1137x699 calibrated window canvas with spring-animated show/hide transitions.
- AMOLED Black (#000000), Graphite Dark, and Porcelain Light theme engine.
- Canonical primary brand accent #9998F6 (RGB: 153, 152, 246).
- 5-layer luminance surface hierarchy preventing component blending.
- 2,078 on-demand high-DPI rasterized SVG icons via LunaSVG dynamic atlas.
- Tactile widgets: Pill switches, checkboxes, single & range sliders, dropdowns, multi-select, color pickers, password inputs, keybinds (keyboard + M1-M5).
- Atomic JSON configuration engine with debounced auto-save.
- Asynchronous developer key verification system.
- Standalone DirectX 11 host window or hook into existing DX11 Present swap chain.

## Documentation Index
- [Getting Started](markdown/getting-started.md): Installation, requirements, CMake linking, minimal app.
- [Core Architecture](markdown/core-api.md): Initialization, DPI, window shell, navigation, subtabs, grids, groupboxes.
- [Widget Catalog](markdown/widgets.md): Complete widget signatures, params, callbacks, return values.
- [Appearance & Themes](markdown/appearance.md): AMOLED Black, 5-layer surface tokens, #9998F6 accent, Inter fonts.
- [Advanced Systems](markdown/advanced.md): JSON config persistence, key system, search spotlight, host vs hook.
- [SVG Icon Catalog](markdown/icons.md): Referencing 2,078 icons by slug, on-demand atlas, tinting.
- [API Reference](markdown/reference.md): Public headers, structs, classes, enums, constants.
- [AI Agent Guide](markdown/ai-agent-guide.md): Critical rules, pitfalls, copy-paste boilerplate.

## Primary Entry Point Header
```cpp
#include <parallax/parallax.hpp>
```

## CMake Linking Target
```cmake
find_package(Parallax REQUIRED)
target_link_libraries(my_app PRIVATE Parallax::Parallax)
```
"""
    with open(os.path.join(DOCS_DIR, "llms.txt"), "w", encoding="utf-8") as f:
        f.write(llms_txt)

    # llms-full.txt (combines all markdown docs)
    full_parts = [
        "# PARALLAX UI — COMPLETE SPECIFICATION & API REFERENCE (v1.2.0)",
        "Version: 1.2.0 | Language: C++20 | Backend: DirectX 11 & Dear ImGui v1.93.0",
        "=" * 80,
        ""
    ]
    for fname, content in docs.items():
        full_parts.append(f"\n<!-- SECTION: {fname} -->\n")
        full_parts.append(content)
        full_parts.append("\n" + "=" * 80 + "\n")

    with open(os.path.join(DOCS_DIR, "llms-full.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(full_parts))

    print("  [OK] Created llms.txt and llms-full.txt")

def generate_html_website(docs, catalog):
    print("[4/5] Building static HTML/CSS/JS documentation website...")

    # Embed docs content as a JSON object into the static HTML for instant client-side rendering
    docs_json = json.dumps(docs)
    catalog_json = json.dumps(catalog[:300]) # First 300 icons for instant display, rest search on demand

    html_content = f"""<!DOCTYPE html>
<html lang="en" data-theme="amoled">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Parallax UI — C++20 DirectX 11 Interface Documentation</title>
    <link rel="icon" type="image/svg+xml" href="../assets/logo.svg">
    <!-- Inter Font -->
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500;600&display=swap" rel="stylesheet">
    <!-- Prism.js Syntax Highlighting -->
    <link href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-tomorrow.min.css" rel="stylesheet" />
    <style>
        :root {{
            --bg-base: #000000;
            --bg-header: #050507;
            --bg-sidebar: #07080a;
            --bg-surface: #0a0b0e;
            --bg-card: #0f1015;
            --bg-widget: #14161c;
            --bg-hover: #1c1f28;
            --border-subtle: rgba(255, 255, 255, 0.08);
            --border-hover: rgba(255, 255, 255, 0.16);
            --text-primary: #f5f6fa;
            --text-secondary: #9ea3b5;
            --text-muted: #62677b;
            --accent-primary: #9998F6;
            --accent-glow: rgba(153, 152, 246, 0.25);
            --accent-hover: #b0aff8;
            --code-bg: #07080b;
            --sidebar-width: 280px;
        }}

        [data-theme="light"] {{
            --bg-base: #f8f9fa;
            --bg-header: #ffffff;
            --bg-sidebar: #ffffff;
            --bg-surface: #f1f3f5;
            --bg-card: #ffffff;
            --bg-widget: #e9ecef;
            --bg-hover: #dee2e6;
            --border-subtle: rgba(0, 0, 0, 0.08);
            --border-hover: rgba(0, 0, 0, 0.16);
            --text-primary: #1a1a24;
            --text-secondary: #495057;
            --text-muted: #868e96;
            --accent-primary: #7574ea;
            --accent-glow: rgba(117, 116, 234, 0.20);
            --accent-hover: #5d5cdb;
            --code-bg: #f1f3f5;
        }}

        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}

        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background-color: var(--bg-base);
            color: var(--text-primary);
            line-height: 1.6;
            display: flex;
            height: 100vh;
            overflow: hidden;
        }}

        /* Sidebar Navigation */
        .sidebar {{
            width: var(--sidebar-width);
            background: var(--bg-sidebar);
            border-right: 1px solid var(--border-subtle);
            display: flex;
            flex-direction: column;
            flex-shrink: 0;
            height: 100%;
        }}

        .brand {{
            padding: 20px 24px;
            display: flex;
            align-items: center;
            gap: 12px;
            border-bottom: 1px solid var(--border-subtle);
        }}

        .brand img {{
            width: 32px;
            height: 32px;
        }}

        .brand-title {{
            font-size: 17px;
            font-weight: 700;
            color: var(--text-primary);
            letter-spacing: -0.02em;
        }}

        .brand-version {{
            font-size: 11px;
            padding: 2px 6px;
            background: rgba(153, 152, 246, 0.15);
            color: var(--accent-primary);
            border-radius: 4px;
            font-weight: 600;
            margin-left: auto;
        }}

        .nav-list {{
            overflow-y: auto;
            padding: 16px 12px;
            flex: 1;
        }}

        .nav-category {{
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.08em;
            color: var(--text-muted);
            padding: 12px 12px 6px;
            font-weight: 600;
        }}

        .nav-item {{
            display: flex;
            align-items: center;
            gap: 10px;
            padding: 8px 12px;
            border-radius: 6px;
            color: var(--text-secondary);
            text-decoration: none;
            font-size: 13.5px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.15s ease;
            margin-bottom: 2px;
        }}

        .nav-item:hover {{
            background: var(--bg-hover);
            color: var(--text-primary);
        }}

        .nav-item.active {{
            background: rgba(153, 152, 246, 0.12);
            color: var(--accent-primary);
            font-weight: 600;
        }}

        /* Main Workspace Container */
        .main-container {{
            flex: 1;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            background: var(--bg-surface);
        }}

        /* Top Bar */
        .topbar {{
            height: 56px;
            background: var(--bg-header);
            border-bottom: 1px solid var(--border-subtle);
            display: flex;
            align-items: center;
            padding: 0 32px;
            gap: 16px;
        }}

        .search-box {{
            flex: 1;
            max-width: 480px;
            position: relative;
        }}

        .search-input {{
            width: 100%;
            height: 36px;
            background: var(--bg-widget);
            border: 1px solid var(--border-subtle);
            border-radius: 6px;
            padding: 0 12px 0 36px;
            color: var(--text-primary);
            font-size: 13.5px;
            outline: none;
            transition: border-color 0.15s ease, box-shadow 0.15s ease;
        }}

        .search-input:focus {{
            border-color: var(--accent-primary);
            box-shadow: 0 0 0 2px var(--accent-glow);
        }}

        .search-icon {{
            position: absolute;
            left: 12px;
            top: 10px;
            width: 16px;
            height: 16px;
            fill: none;
            stroke: var(--text-muted);
            stroke-width: 2;
        }}

        .topbar-actions {{
            display: flex;
            align-items: center;
            gap: 12px;
            margin-left: auto;
        }}

        .btn-icon {{
            background: var(--bg-widget);
            border: 1px solid var(--border-subtle);
            color: var(--text-secondary);
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: 500;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            transition: all 0.15s ease;
        }}

        .btn-icon:hover {{
            background: var(--bg-hover);
            color: var(--text-primary);
            border-color: var(--border-hover);
        }}

        /* Content Area */
        .content-scroll {{
            flex: 1;
            overflow-y: auto;
            padding: 40px 60px 80px;
        }}

        .doc-content {{
            max-width: 900px;
            margin: 0 auto;
        }}

        .doc-content h1 {{
            font-size: 32px;
            font-weight: 700;
            letter-spacing: -0.03em;
            margin-bottom: 12px;
            color: var(--text-primary);
        }}

        .doc-content h2 {{
            font-size: 22px;
            font-weight: 600;
            letter-spacing: -0.02em;
            margin: 36px 0 16px;
            padding-bottom: 8px;
            border-bottom: 1px solid var(--border-subtle);
            color: var(--text-primary);
        }}

        .doc-content h3 {{
            font-size: 17px;
            font-weight: 600;
            margin: 24px 0 10px;
            color: var(--accent-primary);
        }}

        .doc-content p {{
            margin-bottom: 16px;
            color: var(--text-secondary);
            font-size: 14.5px;
        }}

        .doc-content ul, .doc-content ol {{
            margin: 0 0 16px 24px;
            color: var(--text-secondary);
            font-size: 14.5px;
        }}

        .doc-content li {{
            margin-bottom: 6px;
        }}

        .doc-content table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            font-size: 13.5px;
        }}

        .doc-content th, .doc-content td {{
            padding: 10px 14px;
            border: 1px solid var(--border-subtle);
            text-align: left;
        }}

        .doc-content th {{
            background: var(--bg-card);
            color: var(--text-primary);
            font-weight: 600;
        }}

        .doc-content tr:nth-child(even) {{
            background: rgba(255, 255, 255, 0.02);
        }}

        .doc-content code:not(pre code) {{
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            padding: 2px 6px;
            border-radius: 4px;
            font-family: 'JetBrains Mono', monospace;
            font-size: 13px;
            color: var(--accent-primary);
        }}

        .code-block-container {{
            position: relative;
            margin: 20px 0;
            border-radius: 8px;
            overflow: hidden;
            border: 1px solid var(--border-subtle);
            background: var(--code-bg);
        }}

        .code-block-header {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 8px 16px;
            background: rgba(255, 255, 255, 0.03);
            border-bottom: 1px solid var(--border-subtle);
            font-size: 12px;
            color: var(--text-muted);
            font-family: 'JetBrains Mono', monospace;
        }}

        .copy-btn {{
            background: transparent;
            border: 1px solid var(--border-subtle);
            color: var(--text-secondary);
            padding: 3px 8px;
            border-radius: 4px;
            font-size: 11px;
            cursor: pointer;
            transition: all 0.15s ease;
        }}

        .copy-btn:hover {{
            background: var(--bg-hover);
            color: var(--text-primary);
        }}

        pre[class*="language-"] {{
            margin: 0;
            padding: 16px;
            background: transparent !important;
            font-family: 'JetBrains Mono', monospace;
            font-size: 13px;
        }}

        /* Icon Explorer Grid */
        .icon-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(130px, 1fr));
            gap: 12px;
            margin-top: 20px;
        }}

        .icon-card {{
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: 6px;
            padding: 16px 8px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 8px;
            cursor: pointer;
            transition: all 0.15s ease;
            text-align: center;
        }}

        .icon-card:hover {{
            border-color: var(--accent-primary);
            box-shadow: 0 4px 12px var(--accent-glow);
            transform: translateY(-2px);
        }}

        .icon-card svg {{
            width: 24px;
            height: 24px;
            stroke: var(--text-primary);
            fill: none;
            stroke-width: 2;
        }}

        .icon-slug {{
            font-size: 11.5px;
            color: var(--text-muted);
            word-break: break-all;
            font-family: 'JetBrains Mono', monospace;
        }}

        /* Toast Popup */
        #toast {{
            position: fixed;
            bottom: 24px;
            right: 24px;
            background: var(--bg-card);
            border: 1px solid var(--accent-primary);
            color: var(--text-primary);
            padding: 12px 20px;
            border-radius: 6px;
            box-shadow: 0 8px 24px rgba(0, 0, 0, 0.4);
            font-size: 13px;
            opacity: 0;
            pointer-events: none;
            transition: opacity 0.2s ease, transform 0.2s ease;
            transform: translateY(8px);
            z-index: 1000;
        }}

        #toast.show {{
            opacity: 1;
            transform: translateY(0);
        }}
    </style>
</head>
<body>

    <!-- Sidebar Navigation -->
    <aside class="sidebar">
        <div class="brand">
            <img src="../assets/logo.svg" alt="Parallax Logo">
            <span class="brand-title">PARALLAX UI</span>
            <span class="brand-version">v1.2.0</span>
        </div>

        <nav class="nav-list" id="navList">
            <div class="nav-category">Documentation</div>
            <a class="nav-item active" onclick="loadPage('getting-started.md')">Getting Started</a>
            <a class="nav-item" onclick="loadPage('core-api.md')">Core Architecture</a>
            <a class="nav-item" onclick="loadPage('widgets.md')">Widget Catalog</a>
            <a class="nav-item" onclick="loadPage('appearance.md')">Appearance & Themes</a>
            <a class="nav-item" onclick="loadPage('advanced.md')">Advanced Systems</a>
            <a class="nav-item" onclick="loadPage('reference.md')">Public API Reference</a>
            
            <div class="nav-category">Assets & AI Tools</div>
            <a class="nav-item" onclick="loadIconGallery()">SVG Icon Explorer (2,078)</a>
            <a class="nav-item" onclick="loadPage('ai-agent-guide.md')">AI Agent Integration</a>
            <a class="nav-item" href="llms.txt" target="_blank">llms.txt Spec ↗</a>
            <a class="nav-item" href="llms-full.txt" target="_blank">llms-full.txt Unified ↗</a>
        </nav>
    </aside>

    <!-- Main Content Workspace -->
    <main class="main-container">
        <!-- Top Toolbar -->
        <header class="topbar">
            <div class="search-box">
                <svg class="search-icon" viewBox="0 0 24 24"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                <input type="text" class="search-input" id="searchBox" placeholder="Search documentation, widgets, APIs... (Press / to focus)" onkeyup="handleSearch(this.value)">
            </div>

            <div class="topbar-actions">
                <button class="btn-icon" onclick="toggleTheme()" id="themeToggleBtn">
                    <span id="themeLabel">AMOLED</span>
                </button>
                <a class="btn-icon" href="https://github.com/VPROJECT-max/parallax-imgui" target="_blank">
                    GitHub ↗
                </a>
            </div>
        </header>

        <!-- Scrollable Document Content Area -->
        <div class="content-scroll" id="contentScroll">
            <article class="doc-content" id="docContent">
                <!-- Rendered dynamically -->
            </article>
        </div>
    </main>

    <!-- Toast Notification -->
    <div id="toast">Copied to clipboard!</div>

    <!-- Scripts -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-c.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-cpp.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-cmake.min.js"></script>

    <script>
        const docs = {docs_json};
        const iconsCatalog = {catalog_json};
        let currentPage = 'getting-started.md';

        function showToast(msg) {{
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.classList.add('show');
            setTimeout(() => t.classList.remove('show'), 2200);
        }}

        function copyCode(text) {{
            navigator.clipboard.writeText(text).then(() => {{
                showToast("Copied to clipboard!");
            }});
        }}

        function toggleTheme() {{
            const cur = document.documentElement.getAttribute('data-theme');
            const next = cur === 'amoled' ? 'light' : 'amoled';
            document.documentElement.setAttribute('data-theme', next);
            document.getElementById('themeLabel').textContent = next === 'amoled' ? 'AMOLED' : 'Light';
        }}

        // Simple Markdown parser to avoid external runtime bundlers
        function parseMarkdown(md) {{
            let html = md;

            // Headers
            html = html.replace(/^### (.*$)/gim, '<h3>$1</h3>');
            html = html.replace(/^## (.*$)/gim, '<h2>$1</h2>');
            html = html.replace(/^# (.*$)/gim, '<h1>$1</h1>');

            // Code blocks
            html = html.replace(/```(cpp|cmake|bash)?\\n([\\s\\S]*?)```/gm, function(match, lang, code) {{
                const cleanCode = code.trim();
                const displayLang = lang ? lang.toUpperCase() : 'CODE';
                const escaped = cleanCode.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
                return `<div class="code-block-container">
                    <div class="code-block-header">
                        <span>${{displayLang}}</span>
                        <button class="copy-btn" onclick="copyCode(decodeURIComponent('${{encodeURIComponent(cleanCode)}}'))">Copy</button>
                    </div>
                    <pre><code class="language-${{lang || 'cpp'}}">${{escaped}}</code></pre>
                </div>`;
            }});

            // Inline code
            html = html.replace(/`([^`]+)`/g, '<code>$1</code>');

            // Bold & Italics
            html = html.replace(/\\*\\*(.*?)\\*\\*/g, '<strong>$1</strong>');
            html = html.replace(/\\*(.*?)\\*/g, '<em>$1</em>');

            // Horizontal rule
            html = html.replace(/^---$/gim, '<hr style="border:0; border-top:1px solid var(--border-subtle); margin:28px 0;">');

            // Links
            html = html.replace(/\\[([^\\]]+)\\]\\(([^\\)]+)\\)/g, '<a href="$2" style="color:var(--accent-primary); text-decoration:none;">$1</a>');

            // Unordered Lists
            html = html.replace(/^\\s*-\\s+(.*$)/gim, '<li>$1</li>');
            html = html.replace(/(<li>.*<\\/li>)/s, '<ul>$1</ul>');

            // Paragraphs
            html = html.split('\\n\\n').map(p => {{
                if (p.trim().startsWith('<h') || p.trim().startsWith('<div') || p.trim().startsWith('<ul') || p.trim().startsWith('<table') || p.trim().startsWith('<hr')) {{
                    return p;
                }}
                return `<p>${{p.replace(/\\n/g, '<br>')}}</p>`;
            }}).join('\\n');

            return html;
        }}

        function loadPage(pageName) {{
            currentPage = pageName;
            document.querySelectorAll('.nav-item').forEach(el => {{
                el.classList.remove('active');
                if (el.textContent.toLowerCase().includes(pageName.replace('.md','').replace('-',' '))) {{
                    el.classList.add('active');
                }}
            }});

            const content = docs[pageName] || "# Page Not Found";
            document.getElementById('docContent').innerHTML = parseMarkdown(content);
            document.getElementById('contentScroll').scrollTop = 0;
            Prism.highlightAll();
        }}

        function loadIconGallery() {{
            document.querySelectorAll('.nav-item').forEach(el => el.classList.remove('active'));
            event.target.classList.add('active');

            let html = `<h1>SVG Icon Catalog (2,078 Icons)</h1>
            <p>Parallax embeds 2,078 SVG icons from Lucide. Icons are rasterized on-demand into an efficient Direct3D 11 shelf atlas texture. Click any card to copy its slug.</p>
            <div style="margin: 20px 0;">
                <input type="text" class="search-input" placeholder="Filter icons (e.g. search, settings, arrow, user)..." onkeyup="filterIcons(this.value)">
            </div>
            <div class="icon-grid" id="iconGrid">`;

            iconsCatalog.forEach(ic => {{
                html += `<div class="icon-card" onclick="copyCode('${{ic.slug}}')">
                    <div style="font-size:20px; color:var(--accent-primary);">⚡</div>
                    <span class="icon-slug">${{ic.slug}}</span>
                </div>`;
            }});

            html += `</div>`;
            document.getElementById('docContent').innerHTML = html;
        }}

        function filterIcons(query) {{
            const q = query.toLowerCase();
            const grid = document.getElementById('iconGrid');
            const filtered = iconsCatalog.filter(ic => ic.slug.toLowerCase().includes(q));
            let html = '';
            filtered.forEach(ic => {{
                html += `<div class="icon-card" onclick="copyCode('${{ic.slug}}')">
                    <div style="font-size:20px; color:var(--accent-primary);">⚡</div>
                    <span class="icon-slug">${{ic.slug}}</span>
                </div>`;
            }});
            grid.innerHTML = html;
        }}

        function handleSearch(query) {{
            if (!query.trim()) return;
            const q = query.toLowerCase();
            for (const [page, text] of Object.entries(docs)) {{
                if (text.toLowerCase().includes(q)) {{
                    loadPage(page);
                    break;
                }}
            }}
        }}

        // Keyboard navigation (/ to focus search)
        document.addEventListener('keydown', (e) => {{
            if (e.key === '/' && document.activeElement !== document.getElementById('searchBox')) {{
                e.preventDefault();
                document.getElementById('searchBox').focus();
            }}
        }});

        // Initial Load
        loadPage('getting-started.md');
    </script>
</body>
</html>
"""

    html_path = os.path.join(DOCS_DIR, "index.html")
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(html_content)

    print(f"  [OK] Static documentation website generated at {html_path}")

def build_all():
    print("==========================================================")
    print(" Building Parallax UI Public Documentation Suite")
    print("==========================================================")
    ensure_dirs()
    catalog = generate_icons_catalog()
    docs = generate_markdown_docs()
    generate_llms_files(docs)
    generate_html_website(docs, catalog)
    print("==========================================================")
    print(" Documentation Suite Successfully Generated in docs/")
    print("==========================================================")

if __name__ == "__main__":
    build_all()
