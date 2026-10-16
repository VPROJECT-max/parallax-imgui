# Complete Public API Reference

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
