# Guard against multiple inclusion
if(DEFINED _CRAB_ENGINE_HELPERS_INCLUDED)
    return()
endif()
set(_CRAB_ENGINE_HELPERS_INCLUDED TRUE)

# ----------------------------------------
# Detect DOTNET_ROOT (for scripting interop)
# ----------------------------------------
execute_process(
        COMMAND dotnet --list-sdks
        OUTPUT_VARIABLE DOTNET_SDK_LIST
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "\\[(.*)\\]" _DOTNET_SDK_PATH_LINE "${DOTNET_SDK_LIST}")
string(REGEX REPLACE ".*\\[(.*)\\].*" "\\1" DOTNET_ROOT "${_DOTNET_SDK_PATH_LINE}")
get_filename_component(DOTNET_ROOT "${DOTNET_ROOT}" DIRECTORY)

set(ENV{DOTNET_ROOT} "${DOTNET_ROOT}")
add_compile_definitions(DOTNET_ROOT="${DOTNET_ROOT}")

message(STATUS "Detected DOTNET_ROOT: ${DOTNET_ROOT}")

# ----------------------------------------
# Optional function: Copy engine resources
# ----------------------------------------
function(target_copy_engine_resources TARGET)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CRAB_ENGINE_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:${TARGET}>/Engine/resources"
            COMMENT "Copying CrabEngine resources to output directory"
    )
endfunction()

# ----------------------------------------
# Optional: Copy a file if it exists
# ----------------------------------------
function(target_copy_if_exists TARGET SRC DST)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -DSRC=${SRC}
            -DDST=${DST}
            -P "${CRAB_ENGINE_CMAKE_DIR}/CopyIfExists.cmake"
            COMMENT "Conditionally copying ${SRC} to ${DST}"
    )
endfunction()

function(detect_dotnet)
    execute_process(
            COMMAND dotnet --list-sdks
            OUTPUT_VARIABLE DOTNET_SDK_LIST
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "\\[(.*)\\]" _DOTNET_SDK_PATH_LINE "${DOTNET_SDK_LIST}")
    string(REGEX REPLACE ".*\\[(.*)\\].*" "\\1" DOTNET_ROOT "${_DOTNET_SDK_PATH_LINE}")
    get_filename_component(DOTNET_ROOT "${DOTNET_ROOT}" DIRECTORY)
    message(STATUS "Detected DOTNET_ROOT: ${DOTNET_ROOT}")
    add_compile_definitions(DOTNET_ROOT="${DOTNET_ROOT}")
    set(ENV{DOTNET_ROOT} "${DOTNET_ROOT}")
endfunction()

# ----------------------------------------
# Optional function: Copy engine dependency dll's
# ----------------------------------------
function(target_copy_crabengine_dependencies TARGET)
    # Copy all CrabEngine DLLs to the target's binary directory
    foreach(DLL ${CRAB_ENGINE_DLLS})
        # Use target_copy_if_exists to conditionally copy each DLL
        target_copy_if_exists(
                ${TARGET}
                ${DLL}
                "$<TARGET_FILE_DIR:${TARGET}>"  # Output to target's binary dir
        )
    endforeach()

    # Invoke target_copy_webgpu_binaries on the target project
    target_copy_webgpu_binaries(${TARGET})
endfunction()