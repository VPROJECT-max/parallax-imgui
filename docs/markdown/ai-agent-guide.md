# AI Coding Assistant Integration Guide

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
