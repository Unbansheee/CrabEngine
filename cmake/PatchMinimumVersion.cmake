function(patch_cmake_minimum_version LIBRARY_DIR)
    # Define the path to the original CMakeLists.txt and the patched version
    set(ORIGINAL_CMAKE_FILE "${LIBRARY_DIR}/CMakeLists.txt")
    set(PATCHED_CMAKE_FILE "${CMAKE_BINARY_DIR}/patched_lib/CMakeLists.txt")

    # Read the original CMakeLists.txt content
    file(READ "${ORIGINAL_CMAKE_FILE}" _lib_cmake)

    # Replace the old minimum version with a more recent one
    string(REPLACE "cmake_minimum_required(VERSION 2.8)" "cmake_minimum_required(VERSION 3.5)" _lib_cmake "${_lib_cmake}")

    # Write the patched CMakeLists.txt to the binary directory
    file(WRITE "${PATCHED_CMAKE_FILE}" "${_lib_cmake}")

    # Add the patched directory to the project
    add_subdirectory("${CMAKE_BINARY_DIR}/patched_lib")
endfunction()
