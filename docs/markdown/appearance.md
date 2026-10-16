# Appearance, Themes & Design System

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
