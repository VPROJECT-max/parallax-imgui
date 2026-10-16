# Changelog

All notable changes to **Parallax** are documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) (`MAJOR.MINOR.PATCH`):
- **MAJOR**: Incompatible public API changes.
- **MINOR**: Backward-compatible functionality, themes, widgets, and architectural additions.
- **PATCH**: Backward-compatible bug fixes and visual/layout refinements.

---

## [1.2.0] - 2026-10-16

### Added
- **Pure Black AMOLED Theme Preset (`ThemeMode::Amoled`)**:
  - Calibrated true `#000000` window background paired with a strict 5-layer dark-gray surface hierarchy (`#050507` navigation bar, `#0A0B0E` workspace, `#0F1015` content cards, `#14161C` elevated card headers and control surfaces, `#1C1F28` hover surfaces) so cards and controls maintain crisp boundary definition without merging.
  - High-contrast `#F5F6FA` primary text, `#9E9EB2` secondary text, and `#55566A` muted labels.
  - Set as default showcase theme while preserving **Graphite (Dark)** and **Porcelain (Light)** modes.
  - Integrated 3-way segmented `ThemeSwitcher` toggle (AMOLED / Graphite / Porcelain) with smooth spring-physics active pill.
- **Official Parallax Primary Brand Accent (`#9998F6`)**:
  - Added `AccentPreset::Parallax` (`RGB: 153, 152, 246` / `#9998F6`) as the canonical brand accent, set as default across all themes and controls.
  - Integrated into `AccentSelector` preset bar and persisted via JSON configuration.
- **Global Search Catalog & Interactive Results Popover**:
  - Indexed all tabs, sections, and controls across the application with metadata and icons.
  - Case-insensitive, space-separated token matching in `HeaderWidgets::SearchBar`.
  - Floating results popover with tab badges, metadata subheadings, and hover animations.
  - Selecting a search result navigates to the target tab, automatically expands collapsible cards (`CollapsibleGroupbox`), spotlights the section with an animated pulsing accent outline, and resets the search query without affecting widget states. Pressing `Escape` clears the query.
- **Keybind Simulation & Diagnostics Suite**:
  - Added testable simulation hooks `SimulateKeyState(int vkey, bool isDown)` and `ClearSimulatedKeys()` for headless automated verification.
  - Added first-class `KeybindRow(label, description, bind)` layout helper with full `ItemSize`/`ItemAdd` layout registration.

### Changed
- **Workspace Architecture & Inertial Scrolling**:
  - Decoupled the pinned Workspace Header Bar (title, search bar, theme switcher, stats badge) from the scrollable content canvas. The header bar remains fixed at all times.
  - Isolated content scrolling to a dedicated `##ParallaxTabScrollRegion` with `ImGuiWindowFlags_NoScrollWithMouse` and pre-layout `ImGui::SetNextWindowScroll(ImVec2(0.0f, std::round(s_currentScrollY)))`, eliminating double-wheel fighting, position jumps, and 1-frame height pop on tab switching.
  - Locked child groupbox scroll positions to `(0, 0)` via `SetNextWindowScroll`, eliminating relative drift between cards and their headers when navigating tall menus.
- **Original Parallax SVG Logo Restoration**:
  - Restored the official multi-layer vector emblem from `assets/logo.svg` in both the vertical sidebar and horizontal navigation bar.
  - Hardened `RasterizeSVG` pipeline with un-premultiplied straight RGBA alpha conversion when `preserveColors = true` for pristine DirectX 11 blending.
  - Boosted texture atlas rasterization bounds to `64px * dpiScale` (up to 256px) for razor-sharp rendering on high-DPI displays.

### Fixed
- **Dear ImGui Layout Diagnostics & Boundary Assertions**:
  - Resolved `Code uses SetCursorPos()/SetCursorScreenPos() to extend window/parent boundaries` warnings across `src/widgets/input_text.cpp`, `src/layout/grid.cpp`, `src/layout/profile_card.cpp`, `src/layout/tabs.cpp`, and `src/layout/navbar.cpp` by updating `window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, frameBb.Max)` and explicitly clearing `window->DC.IsSetPos = false` after cursor restoration.
  - Fixed manual `SetCursorPosY` overreach in `examples/demo_dx11/main.cpp` keybind lines by adopting `KeybindRow`.
  - Fixed multi-cell cursor advancement in SVG icon atlas grid cells by allocating a single bounding dummy.
- **Keybind Capture & Shortcut Conflicts**:
  - Added release-gating to prevent instant binding of the initial click event.
  - Supported mouse buttons (M1–M5) and all keyboard keys with `Esc` to cancel and `Del`/`Backspace` to clear.
  - Edge-tracked `m_wasKeyDown` per-frame during key capture in `HotKeyManager`, eliminating accidental menu toggles when binding `VK_RSHIFT` or `VK_INSERT`.
  - Switched keybind edge detection to per-`Keybind*` state mapping in `UpdateKeybindState`, eliminating cross-instance toggling bugs.

---

## [1.1.0] - 2026-10-16

### Added
- **Semantic Versioning Macros**: Added `PARALLAX_VERSION_MAJOR` (`1`), `PARALLAX_VERSION_MINOR` (`1`), `PARALLAX_VERSION_PATCH` (`0`), and `PARALLAX_VERSION_STRING` (`"1.1.0"`) in `<parallax/core/types.hpp>`, synchronized with `CMakeLists.txt`, the sidebar brand block, and showcase headers.
- **Centralized Design Token System (`StyleColors` & `StyleMetrics`)**:
  - 5-layer surface hierarchy: `WindowBg` (Application Shell), `HeaderBg` (Navigation Surface), `ContentBg` (Workspace Canvas), `CardBg` / `CardHeaderBg` (Elevated Content Cards), and `WidgetBg` / `WidgetHover` / `WidgetActive` (Interactive Control Surfaces), plus `PopupBg`, `OverlayDim`, `Shadow`, and 3-tier borders (`BorderSubtle`, `Border`, `BorderStrong`).
  - Expanded `StyleMetrics` scale for spacing (`SpaceXS`–`SpaceXL`), typography (`FontTitle`–`FontCaption`), control geometry (`ControlHeight`, `ControlHeightCompact`, `SliderTrackHeight`, `ToggleWidth`, `ToggleHeight`), and animation timing (`AnimFast`, `AnimNormal`, `AnimSlow`).
- **Dual Theme Engine (`Graphite` Dark & `Porcelain` Light)**:
  - **Graphite (`ThemeMode::Dark`, default)**: Deep graphite layered surfaces, soft neutral borders, and readable off-white typography.
  - **Porcelain (`ThemeMode::Light`)**: Dedicated warm off-white base (`#F4F3EF`), pure white elevated cards (`#FFFFFF`), dark graphite text (`#191B1F`), and calibrated subtle shadows.
  - Smooth per-frame color interpolation (`StyleManager::UpdateTransition`) between Dark and Light themes without harsh visual flashes or lost widget state.
  - Dynamic Win32 DWM titlebar theme synchronization and adaptive D3D11 viewport clear color in `D3D11Host`.
- **Restraint-First Accent Engine**:
  - Replaced legacy neon purple default with **Muted Sage (`#78A18B`, `AccentPreset::Sage`)**.
  - Added built-in contrast-safe accent presets: `Sage`, `Monochrome`, `Copper`, `SteelBlue`, and `Custom` user-defined RGBA accents, automatically deriving hover, active, subtle fill, and glow tokens for both Dark and Light themes.
- **Public Theme & Accent API (`<parallax/parallax.hpp>`)**:
  - `SetTheme(ThemeMode)`, `GetTheme()`, `ToggleTheme()`, `IsLightTheme()`
  - `SetAccentPreset(AccentPreset)`, `GetAccentPreset()`, `SetCustomAccent(const ImVec4&)`, `GetAccentColor()`
  - `ThemeSwitcher(const char*)` and `AccentSelector(const char*)` header/toolbar controls.
- **Custom Widget Additions & Redesigns**:
  - **Toggles & Checkboxes**: Added `Toggle`, `ToggleCompact`, and `CheckboxIndeterminate`, plus `ControlRow::Description`, `ControlRow::Disabled`, `ControlRow::Compact`, and `ControlRow::Indeterminate`.
  - **Buttons**: Added `ButtonVariant::Destructive` (`Danger` retained as backward-compatible alias), `ButtonCompact`, `IconButton`, and `ButtonEx(label, ButtonOptions)` with loading spinner (`options.loading`), disabled state, and trailing shortcut hint badge.
  - **Sliders & Dual Range Sliders**: Redesigned `SliderFloat`, `SliderInt`, `RangeSliderFloat`, and `RangeSliderInt` with machined pill thumbs, center grip notches, floating value callouts, tick marks (`SliderFloatEx` with `SliderOptions`), and **double-click / Ctrl+Click inline numeric editing**.
  - **Color Pickers**: Added `ColorPickerRow(label, color, hasAlpha)` with inline hex readout badge, preset palette swatches inside the popup, and theme-adaptive popover styling.
  - **Auto-Sizing Groupboxes**: `Groupbox::Begin(..., height = 0.0f)` now automatically fits its child height to its contents via `ImGuiChildFlags_AutoResizeY` while supporting explicit fixed heights when `height > 0.0f`. Added `Groupbox::Description` (`parallax::GroupboxDescription`) for secondary card subtitles.
- **Configuration Persistence for Appearance**:
  - `ConfigManager` now persists and restores `theme.mode`, `theme.accent_preset`, and `theme.custom_accent` across JSON disk profiles and Base64 share strings.

### Changed
- **Visual Hierarchy**: Eliminated hardcoded dark RGBA literals across `navbar.cpp`, `tabs.cpp`, `profile_card.cpp`, `dropdown.cpp`, `input_text.cpp`, `keybind.cpp`, `collapsible_groupbox.cpp`, `notification.cpp`, `watermark.cpp`, and `key_system.cpp` so every surface, icon, and text element responds dynamically to active theme and accent tokens.
- **Vector Brand Emblem**: Replaced the static purple bitmap logo in the sidebar header with an accent-adaptive vector emblem.
- **Reference Showcase (`examples/demo_dx11/main.cpp`)**: Reorganized into 7 structured tabs (`Components`, `Selectors`, `Appearance`, `Layouts`, `Icons`, `Configs`, `Key System`) with a clean header bar housing the live `AccentSelector`, `ThemeSwitcher`, and `ProfileCard`.

### Fixed
- Eliminated Dear ImGui `IsSetPos` boundary extension warnings when restoring cursor positions after rendering inline sub-widgets in `ControlRow::Render` and `ColorPickerRow`.
- Fixed unreadable text and untinted SVG icons when switching to light backgrounds.

---

## [1.0.0] - 2026-03-19

### Added
- Initial release of the Parallax C++20 / DirectX 11 / Dear ImGui UI framework.
- Fixed-size (`1137 × 699`) application shell with collapsible navigation sidebar (`220px` expanded / `64px` icon-only) and horizontal/vertical sub-tabs.
- Dynamic SVG icon rasterization pipeline (`IconManager`) powered by LunaSVG with automatic texture atlas packing and per-monitor DPI awareness.
- Spring physics (`Spring`) and cubic/quartic easing (`Easing`) animation primitives.
- Core widget suite: `Checkbox`, `ControlRow`, `SliderFloat`, `SliderInt`, `RangeSliderFloat`, `RangeSliderInt`, `Dropdown`, `MultiDropdown`, `ColorPicker`, `InputText`, `KeybindWidget`, `Button`, `CollapsibleGroupbox`, `NotificationManager`, and `Watermark`.
- JSON configuration engine (`ConfigManager`) with atomic file writes and Base64 import/export.
- Asynchronous key verification module (`KeySystem`) with DPAPI-backed local credential caching.
