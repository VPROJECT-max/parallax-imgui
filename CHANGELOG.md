# Changelog

All notable changes to **Parallax** are documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) (`MAJOR.MINOR.PATCH`):
- **MAJOR**: Incompatible public API changes.
- **MINOR**: Backward-compatible functionality, themes, widgets, and architectural additions.
- **PATCH**: Backward-compatible bug fixes and visual/layout refinements.

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
