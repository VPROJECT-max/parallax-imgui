cmake_minimum_required(VERSION 3.20)
project(parallax_standalone_consumer VERSION 1.0.0 LANGUAGES CXX C)

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

# -----------------------------------------------------------------------------
# Locate Parallax Standalone SDK Package
# -----------------------------------------------------------------------------
set(PARALLAX_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../sdk" CACHE PATH "Path to Parallax Standalone SDK")
list(APPEND CMAKE_PREFIX_PATH "${PARALLAX_SDK_DIR}")

find_package(Parallax REQUIRED)

# -----------------------------------------------------------------------------
# Standalone Consumer Executable (Compiled ONLY against public SDK headers & lib)
# -----------------------------------------------------------------------------
add_executable(consumer_app
    main.cpp
)

target_link_libraries(consumer_app PRIVATE
    Parallax::Parallax
)

# Copy Parallax SDK runtime assets to output directory
add_custom_command(TARGET consumer_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PARALLAX_ASSETS_DIR}/fonts"
        "$<TARGET_FILE_DIR:consumer_app>/assets/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PARALLAX_ASSETS_DIR}/icons"
        "$<TARGET_FILE_DIR:consumer_app>/icons"
    COMMENT "Copying Parallax SDK runtime assets next to consumer binary"
)
