cmake_minimum_required(VERSION 3.0...3.25)
project(
    CrabEngine # name of the project, which will also be the name of the visual studio solution if you use it
    VERSION 0.1.0 # any version number
    LANGUAGES CXX C # programming languages used by the project
)

file(GLOB SOURCES
        src/*.h
        src/*.c
        src/*.hpp
        src/*.cpp
)

add_library(CrabEngine STATIC
        ${SOURCES}
)

set_target_properties(CrabEngine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR OFF
)

if (MSVC)
    target_compile_options(CrabEngine PRIVATE /W4)
else()
    target_compile_options(CrabEngine PRIVATE -Wall -Wextra -pedantic)
endif()

add_subdirectory(vendor/webgpu)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glfw3webgpu)
add_subdirectory(vendor/imgui)
target_link_libraries(CrabEngine PRIVATE glfw glfw3webgpu)
target_link_libraries(CrabEngine PUBLIC webgpu imgui)
target_copy_webgpu_binaries(CrabEngine)
set_target_properties(CrabEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

set_target_properties(glfw PROPERTIES FOLDER "CrabEngine/Dependencies/GLFW3")
set_target_properties(glfw3webgpu PROPERTIES FOLDER "CrabEngine/Dependencies/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "CrabEngine/Dependencies/GLFW3")
set_target_properties(imgui PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(CrabEngine PROPERTIES FOLDER "CrabEngine")


# We add an option to enable different settings when developing the CrabEngine than
# when distributing it.
option(DEV_MODE "Set up development helper settings" ON)

if(DEV_MODE)
    # In dev mode, we load resources from the source tree, so that when we
    # dynamically edit resources (like shaders), these are correctly
    # versionned.
    target_compile_definitions(CrabEngine PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
else()
    # In release mode, we just load resources relatively to wherever the
    # executable is launched from, so that the binary is portable
    target_compile_definitions(CrabEngine PRIVATE
            RESOURCE_DIR="./resources"
    )
endif()

target_compile_definitions(CrabEngine PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(CrabEngine PRIVATE GLM_FORCE_LEFT_HANDED)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU)

if (MSVC)
    # Disable warning C4244: conversion from 'int' to 'short', possible loss of data
    target_compile_options(CrabEngine PUBLIC /wd4244)
    # Disable 4100 - unreferenced parameter
    target_compile_options(CrabEngine PUBLIC /wd4100)
    # Disable 4201 - nameless struct/union (because of GLM)
    target_compile_options(CrabEngine PUBLIC /wd4201)   

    target_compile_options(CrabEngine PUBLIC /MP)
endif (MSVC)

target_include_directories(CrabEngine PRIVATE .)
target_include_directories(CrabEngine PUBLIC ./vendor)
target_include_directories(CrabEngine PUBLIC ./src)


