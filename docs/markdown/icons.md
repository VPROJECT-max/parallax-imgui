# SVG Icon System & Texture Atlas

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
