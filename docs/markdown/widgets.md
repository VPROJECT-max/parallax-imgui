# Widget Catalog & Component API

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
