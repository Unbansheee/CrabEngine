# Read CMake variables passed via -D
set(SOURCE_FILE "${SRC}")
set(DEST_DIR "${DST}")

if(EXISTS "${SOURCE_FILE}")
    message(STATUS "Copying ${SOURCE_FILE} to ${DEST_DIR}")
    file(COPY "${SOURCE_FILE}" DESTINATION "${DEST_DIR}")
else()
    message(STATUS "File ${SOURCE_FILE} does not exist; skipping copy.")
endif()