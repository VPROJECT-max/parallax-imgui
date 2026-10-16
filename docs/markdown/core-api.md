# Core Architecture & Lifecycle API

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
