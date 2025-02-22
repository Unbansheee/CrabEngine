cmake_minimum_required(VERSION 3.0...3.25)
project(
    CrabEditor # name of the project, which will also be the name of the visual studio solution if you use it
    VERSION 0.1.0 # any version number
    LANGUAGES CXX C # programming languages used by the project
)

add_executable(App
        implementations.cpp
        main.cpp
        Application.cpp
        Application.h
        ResourceManager.h
        ResourceManager.cpp
        )

set_target_properties(App PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    target_compile_options(App PRIVATE /W4)
else()
    target_compile_options(App PRIVATE -Wall -Wextra -pedantic)
endif()

add_subdirectory(webgpu)
add_subdirectory(glfw)
add_subdirectory(glfw3webgpu)
add_subdirectory(imgui)
target_link_libraries(App PRIVATE webgpu)
target_copy_webgpu_binaries(App)
target_link_libraries(App PRIVATE webgpu glfw)
target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)
target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu imgui)
target_copy_webgpu_binaries(App)

# We add an option to enable different settings when developing the app than
# when distributing it.
option(DEV_MODE "Set up development helper settings" ON)

if(DEV_MODE)
    # In dev mode, we load resources from the source tree, so that when we
    # dynamically edit resources (like shaders), these are correctly
    # versionned.
    target_compile_definitions(App PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
else()
    # In release mode, we just load resources relatively to wherever the
    # executable is launched from, so that the binary is portable
    target_compile_definitions(App PRIVATE
            RESOURCE_DIR="./resources"
    )
endif()

target_compile_definitions(App PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(App PRIVATE GLM_FORCE_LEFT_HANDED)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU)

if (MSVC)
    # Disable warning C4244: conversion from 'int' to 'short', possible loss of data
    target_compile_options(App PUBLIC /wd4244)
endif (MSVC)

target_include_directories(App PRIVATE .)

