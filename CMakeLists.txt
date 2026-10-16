cmake_minimum_required(VERSION 3.20)
project(parallax_project VERSION 1.2.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(
        /W4
        /permissive-
        /utf-8
        /Zc:__cplusplus
        /MP
        /D_CRT_SECURE_NO_WARNINGS
        /DNOMINMAX
        /DWIN32_LEAN_AND_MEAN
    )
endif()

include(FetchContent)

# -----------------------------------------------------------------------------
# 1. Dear ImGui
# -----------------------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.93.0-docking
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC d3d11 dxgi d3dcompiler)
    if(MSVC)
        target_compile_options(imgui PRIVATE /W0)
    endif()
endif()

# -----------------------------------------------------------------------------
# 2. LunaSVG with recursive submodules (plutovg)
# -----------------------------------------------------------------------------
set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "Disable lunasvg examples" FORCE)
set(PLUTOVG_BUILD_EXAMPLES OFF CACHE BOOL "Disable plutovg examples" FORCE)

FetchContent_Declare(
    lunasvg
    GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
    GIT_TAG        v3.5.0
    GIT_SUBMODULES_RECURSE TRUE
)
FetchContent_MakeAvailable(lunasvg)
if(MSVC AND TARGET lunasvg)
    target_compile_options(lunasvg PRIVATE /W0)
endif()

# -----------------------------------------------------------------------------
# 2.1. nlohmann/json for Configuration Engine
# -----------------------------------------------------------------------------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# -----------------------------------------------------------------------------
# 3. Parallax Static Library
# -----------------------------------------------------------------------------
file(GLOB_RECURSE PARALLAX_HEADERS CONFIGURE_DEPENDS "include/parallax/*.hpp")
file(GLOB_RECURSE PARALLAX_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(parallax STATIC
    ${PARALLAX_HEADERS}
    ${PARALLAX_SOURCES}
)

target_include_directories(parallax PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(parallax PUBLIC
    imgui
    lunasvg::lunasvg
    nlohmann_json::nlohmann_json
    d3d11
    dxgi
    d3dcompiler
    dwmapi
)

# -----------------------------------------------------------------------------
# 4. Standalone DirectX 11 Demo Executable
# -----------------------------------------------------------------------------
add_executable(parallax_demo_dx11
    examples/demo_dx11/main.cpp
)

target_link_libraries(parallax_demo_dx11 PRIVATE
    parallax
)

add_executable(test_headless tests/headless_test.cpp)
target_link_libraries(test_headless PRIVATE parallax)

# Copy icons and assets to the output directory after build
add_custom_command(TARGET parallax_demo_dx11 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/icons"
        "$<TARGET_FILE_DIR:parallax_demo_dx11>/icons"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:parallax_demo_dx11>/assets"
    COMMENT "Copying 2,077 SVG icons and assets to output directory"
)
