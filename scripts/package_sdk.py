#!/usr/bin/env python3
"""
Parallax UI SDK Packaging Script
Creates a clean, self-contained standalone SDK distribution.
"""

import os
import sys
import shutil
import subprocess
import glob

def package_sdk(build_dir="build", output_dir="sdk", config="Release"):
    print("==========================================================")
    print(f" Packaging Parallax UI v1.2.0 Standalone SDK ({config})")
    print("==========================================================")

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    out_path = os.path.join(root_dir, output_dir)

    # 1. Clean output directory
    if os.path.exists(out_path):
        shutil.rmtree(out_path)

    os.makedirs(os.path.join(out_path, "include", "parallax"), exist_ok=True)
    os.makedirs(os.path.join(out_path, "include", "imgui"), exist_ok=True)
    os.makedirs(os.path.join(out_path, "lib", "modular"), exist_ok=True)
    os.makedirs(os.path.join(out_path, "assets", "fonts"), exist_ok=True)
    os.makedirs(os.path.join(out_path, "assets", "icons"), exist_ok=True)
    os.makedirs(os.path.join(out_path, "cmake"), exist_ok=True)

    # 2. Copy Public Parallax Headers
    print("[1/6] Copying public Parallax C++ headers...")
    inc_src = os.path.join(root_dir, "include", "parallax")
    inc_dst = os.path.join(out_path, "include", "parallax")
    shutil.copytree(inc_src, inc_dst, dirs_exist_ok=True)

    # 3. Copy ImGui Headers from fetchcontent
    print("[2/6] Copying Dear ImGui public headers...")
    imgui_src = os.path.join(root_dir, build_dir, "_deps", "imgui-src")
    imgui_dst = os.path.join(out_path, "include", "imgui")
    imgui_headers = ["imgui.h", "imconfig.h", "imgui_internal.h", "imstb_rectpack.h", "imstb_textedit.h", "imstb_truetype.h"]
    for hdr in imgui_headers:
        src_file = os.path.join(imgui_src, hdr)
        if os.path.exists(src_file):
            shutil.copy2(src_file, imgui_dst)
        else:
            print(f"Warning: Missing ImGui header {src_file}")

    # 4. Copy Assets (Fonts, Logo, Icons)
    print("[3/6] Copying assets (fonts, logo, 2,077 SVG icons)...")
    shutil.copytree(os.path.join(root_dir, "assets", "fonts"), os.path.join(out_path, "assets", "fonts"), dirs_exist_ok=True)
    shutil.copy2(os.path.join(root_dir, "assets", "logo.svg"), os.path.join(out_path, "assets", "logo.svg"))
    shutil.copytree(os.path.join(root_dir, "icons"), os.path.join(out_path, "assets", "icons"), dirs_exist_ok=True)

    # 5. Build Unified Standalone Static Library (parallax.lib)
    print("[4/6] Creating unified standalone parallax.lib...")
    parallax_lib = os.path.join(root_dir, build_dir, config, "parallax.lib")
    imgui_lib = os.path.join(root_dir, build_dir, config, "imgui.lib")
    luna_lib = os.path.join(root_dir, build_dir, "_deps", "lunasvg-build", config, "lunasvg.lib")
    pluto_lib = os.path.join(root_dir, build_dir, "_deps", "lunasvg-build", "plutovg", config, "plutovg.lib")

    for p in [parallax_lib, imgui_lib, luna_lib, pluto_lib]:
        if not os.path.exists(p):
            raise FileNotFoundError(f"Missing build library: {p}")

    # Copy modular
    shutil.copy2(parallax_lib, os.path.join(out_path, "lib", "modular", "parallax_core.lib"))
    shutil.copy2(imgui_lib, os.path.join(out_path, "lib", "modular", "imgui.lib"))
    shutil.copy2(luna_lib, os.path.join(out_path, "lib", "modular", "lunasvg.lib"))
    shutil.copy2(pluto_lib, os.path.join(out_path, "lib", "modular", "plutovg.lib"))

    # Find MSVC lib.exe
    lib_exe = r"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\lib.exe"
    if not os.path.exists(lib_exe):
        candidates = glob.glob(r"C:\Program Files\Microsoft Visual Studio\**\Hostx64\x64\lib.exe", recursive=True)
        if candidates:
            lib_exe = candidates[0]
        else:
            lib_exe = None

    unified_lib = os.path.join(out_path, "lib", "parallax.lib")
    if lib_exe and os.path.exists(lib_exe):
        print(f"  Using MSVC Library Manager: {lib_exe}")
        cmd = [lib_exe, "/NOLOGO", f"/OUT:{unified_lib}", parallax_lib, imgui_lib, luna_lib, pluto_lib]
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            print(res.stderr)
            raise RuntimeError("Failed to combine static libraries.")
        size_mb = os.path.getsize(unified_lib) / (1024 * 1024)
        print(f"  [OK] Unified parallax.lib created successfully ({size_mb:.2f} MB)")
    else:
        print("  Warning: lib.exe not found. Copying raw parallax.lib.")
        shutil.copy2(parallax_lib, unified_lib)

    # 6. Generate CMake Package Configuration
    print("[5/6] Generating ParallaxConfig.cmake & ParallaxConfigVersion.cmake...")
    config_cmake = """# Parallax UI CMake Package Configuration
# Defines imported target Parallax::Parallax

cmake_minimum_required(VERSION 3.20)

get_filename_component(PARALLAX_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT TARGET Parallax::Parallax)
    add_library(Parallax::Parallax STATIC IMPORTED)
    
    set_target_properties(Parallax::Parallax PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PARALLAX_SDK_ROOT}/include;${PARALLAX_SDK_ROOT}/include/imgui;${PARALLAX_SDK_ROOT}/include/parallax"
        IMPORTED_LOCATION "${PARALLAX_SDK_ROOT}/lib/parallax.lib"
        INTERFACE_LINK_LIBRARIES "d3d11;dxgi;d3dcompiler;dwmapi;windowscodecs"
        INTERFACE_COMPILE_FEATURES "cxx_std_20"
    )
    
    if(MSVC)
        target_compile_options(Parallax::Parallax INTERFACE /utf-8 /Zc:__cplusplus /DNOMINMAX /DWIN32_LEAN_AND_MEAN)
    endif()
endif()

set(Parallax_FOUND TRUE)
set(PARALLAX_INCLUDE_DIRS "${PARALLAX_SDK_ROOT}/include")
set(PARALLAX_LIBRARIES Parallax::Parallax)
set(PARALLAX_ASSETS_DIR "${PARALLAX_SDK_ROOT}/assets")
"""
    with open(os.path.join(out_path, "cmake", "ParallaxConfig.cmake"), "w", encoding="utf-8") as f:
        f.write(config_cmake)

    version_cmake = """set(PACKAGE_VERSION "1.2.0")
if("${PACKAGE_VERSION}" VERSION_LESS "${PACKAGE_FIND_VERSION}")
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    if("${PACKAGE_VERSION}" VERSION_EQUAL "${PACKAGE_FIND_VERSION}")
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
endif()
"""
    with open(os.path.join(out_path, "cmake", "ParallaxConfigVersion.cmake"), "w", encoding="utf-8") as f:
        f.write(version_cmake)

    # 7. Generate SDK README
    print("[6/6] Generating SDK README.md...")
    sdk_readme = """# Parallax UI SDK v1.2.0
**C++20 DirectX 11 Interface Architecture & Design System**

The Parallax SDK provides everything you need to integrate Parallax into your own C++ applications without requiring internal implementation sources.

---

## SDK Directory Structure

```
sdk/
├── include/
│   ├── parallax/       # Public Parallax C++ headers
│   │   ├── parallax.hpp
│   │   ├── core/
│   │   ├── layout/
│   │   ├── widgets/
│   │   ├── icons/
│   │   ├── dx11/
│   │   ├── anim/
│   │   ├── config/
│   │   ├── auth/
│   │   └── input/
│   └── imgui/          # Dear ImGui headers (v1.93.0 docking)
├── lib/
│   ├── parallax.lib    # Unified standalone static library (includes ImGui, LunaSVG & PlutoVG)
│   └── modular/        # Individual static libraries for custom link configurations
├── assets/
│   ├── fonts/          # Inter font family (Regular, Medium, SemiBold, Bold)
│   ├── icons/          # 2,077 SVG icon assets
│   └── logo.svg        # Official Parallax brand SVG mark
└── cmake/
    ├── ParallaxConfig.cmake
    └── ParallaxConfigVersion.cmake
```

---

## Quick Start with CMake

Add the SDK to your project's `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_parallax_app LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point CMake to the Parallax SDK directory
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/path/to/sdk")
find_package(Parallax REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE Parallax::Parallax)

# Copy runtime assets next to your executable
add_custom_command(TARGET my_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PARALLAX_ASSETS_DIR}/fonts" "$<TARGET_FILE_DIR:my_app>/assets/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PARALLAX_ASSETS_DIR}/icons" "$<TARGET_FILE_DIR:my_app>/icons"
)
```

---

## Minimal Application (Declarative Builder)

```cpp
#include <parallax/parallax.hpp>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // 1. Initialize DX11 host window
    parallax::dx11::D3D11Host host;
    if (!host.InitStandalone(hInstance, 1280, 800, L"My Parallax Application")) {
        return 1;
    }

    // 2. Initialize Parallax library
    parallax::Init(host.GetDevice(), host.GetContext());

    // 3. Application state
    bool menuOpen = true;
    bool enableFeature = true;
    float volume = 0.75f;

    // 4. Construct Window via WindowBuilder
    parallax::WindowBuilder builder("Control Panel");
    builder.WithTheme(parallax::ThemeMode::Amoled)
           .WithAccent(parallax::AccentPreset::Parallax)
           .AddTab("sliders", "Settings", [&](parallax::TabContext& ctx) {
               ctx.BeginGrid(2);
               ctx.Groupbox("Audio Settings", [&]() {
                   ctx.Toggle("Enable Spatial Audio", &enableFeature);
                   ctx.SliderFloat("Master Volume", &volume, 0.0f, 1.0f);
               }, "volume-2");
               ctx.EndGrid();
           });

    // 5. Run message and rendering loop
    host.RunMainLoop([&]() {
        builder.Render(&menuOpen);
    });

    // 6. Clean shutdown
    parallax::Shutdown();
    host.Shutdown();
    return 0;
}
```

---

## System Requirements
- **OS**: Windows 10 / 11 (64-bit)
- **Compiler**: MSVC 19.30+ (Visual Studio 2022) with C++20 standard (`/std:c++20`)
- **Graphics API**: DirectX 11 Feature Level 11_0+
- **DirectX Libraries**: `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, `dwmapi.lib`, `windowscodecs.lib`
"""
    with open(os.path.join(out_path, "README.md"), "w", encoding="utf-8") as f:
        f.write(sdk_readme)

    print("==========================================================")
    print(f" Parallax UI SDK Package Successfully Created at: {out_path}")
    print("==========================================================")

if __name__ == "__main__":
    package_sdk()
