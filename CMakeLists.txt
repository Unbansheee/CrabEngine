cmake_minimum_required(VERSION 3.28)
project(CrabEngine VERSION 0.1.0 LANGUAGES CXX C)

set(CRAB_ENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE INTERNAL "")
set(CRAB_ENGINE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CRAB_ENGINE_DLLS
        "${CRAB_ENGINE_SOURCE_DIR}/vendor/slang/bin/slang.dll"
        "${CRAB_ENGINE_SOURCE_DIR}/vendor/nethost/nethost.dll"

        CACHE INTERNAL "CrabEngine DLLs to copy"
)

include(${CRAB_ENGINE_SOURCE_DIR}/cmake/CrabEngineHelpers.cmake)
include(${CRAB_ENGINE_SOURCE_DIR}/cmake/AddDotnetSupport.cmake)

file(GLOB_RECURSE SOURCES
        src/*.h
        src/*.hpp
        src/*.c
        src/*.cpp
        vendor/ImGuizmo/ImGuizmo.cpp
        vendor/ImGuizmo/ImGuizmo.h
)

file(GLOB_RECURSE MODULE_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false src/*.cppm)

add_library(CrabEngine STATIC ${SOURCES})


set_target_properties(CrabEngine PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        FOLDER "CrabEngine"
)

target_compile_definitions(CrabEngine PUBLIC
        "$<$<CONFIG:Debug>:_DEBUG>"
        "$<$<CONFIG:Debug>:ENGINE_ENABLE_ASSERTS>"
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_LEFT_HANDED
)

target_link_options(CrabEngine INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wl,--whole-archive CrabEngine -Wl --no-whole-archive>
        $<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:CrabEngine>
)

target_compile_options(CrabEngine PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /wd4244 /wd4100 /wd4201>
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(CrabEngine PRIVATE -fmodules-ts -fmodule-header)
endif()

# Include paths
target_include_directories(CrabEngine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Nodes
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src/Utility
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendor
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/slang
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nativefiledialog-extended/src/include
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nethost/include
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/esfw/include
)

# Link dependencies
add_subdirectory(vendor/webgpu)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glfw3webgpu)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/rocket)
add_subdirectory(vendor/nativefiledialog-extended)
add_subdirectory(vendor/string_id)
add_subdirectory(vendor/jolt)
add_subdirectory(vendor/fmt)
add_subdirectory(vendor/vfspp vfspp_build)
add_subdirectory(vendor/esfw)

foreach(target webgpu imgui glfw ...)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES
                CXX_STANDARD 23
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS OFF
        )
    endif()
endforeach()

find_library(LIB_slang slang ${CMAKE_CURRENT_SOURCE_DIR}/vendor/slang/lib)
find_library(LIB_slangGFX gfx ${CMAKE_CURRENT_SOURCE_DIR}/vendor/slang/lib)
find_library(LIB_nethost nethost ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nethost)

target_link_libraries(CrabEngine
        PRIVATE
        glfw
        glfw3webgpu
        nfd
        Jolt
        ${LIB_slang}
        ${LIB_slangGFX}
        ${LIB_nethost}
        efsw-static
)

target_link_libraries(CrabEngine PUBLIC
        webgpu
        imgui
        foonathan_string_id
        fmt
        vfspp
)

# Visual Studio folder structure
set_target_properties(glfw PROPERTIES FOLDER "CrabEngine/Dependencies/GLFW3")
set_target_properties(glfw3webgpu PROPERTIES FOLDER "CrabEngine/Dependencies/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "CrabEngine/Dependencies/GLFW3")
set_target_properties(imgui PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(nfd PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(foonathan_string_id PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(fmt PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(efsw PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(efsw-static PROPERTIES FOLDER "CrabEngine/Dependencies")
set_target_properties(Jolt PROPERTIES FOLDER "CrabEngine/Dependencies")


# Dotnet detection and propagation
detect_dotnet()

# Development mode
option(DEV_MODE "Set up development helper settings" ON)
if (DEV_MODE)
    target_compile_definitions(CrabEngine PUBLIC ENGINE_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")
else()
    target_compile_definitions(CrabEngine PUBLIC ENGINE_RESOURCE_DIR="./Engine/resources")
endif()

# ImGui-specific backend defs
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_WEBGPU_BACKEND_WGPU)

# C++ module support
target_sources(CrabEngine
        PUBLIC
        FILE_SET "modules"
        TYPE CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES ${MODULE_FILES}
)


add_custom_target(BuildCrabEngineDotNet ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/.nuget
        COMMAND dotnet pack ${CMAKE_CURRENT_SOURCE_DIR}/script/CrabEngine/CrabEngine.csproj -c Release -o ${CMAKE_CURRENT_SOURCE_DIR}/.nuget
        COMMENT "Building CrabEngine C# bindings NuGet package"
)

add_dotnet_project(CrabEngine
        ${CMAKE_SOURCE_DIR}/vendor/CrabEngine/script/CrabEngine/CrabEngine.csproj
        $<TARGET_FILE_DIR:CrabEngine>/Dotnet)

add_dependencies(BuildCrabEngineDotNet Build_CrabEngine_Scripts)
add_dependencies(CrabEngine BuildCrabEngineDotNet)

set_target_properties(BuildCrabEngineDotNet PROPERTIES FOLDER "CrabEngine/Dotnet")
set_target_properties(Build_CrabEngine_Scripts PROPERTIES FOLDER "CrabEngine/Dotnet")
