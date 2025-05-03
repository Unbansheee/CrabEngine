
function(add_dotnet_project TARGET_NAME CSHARP_PROJECT_PATH OUTPUT_DIR)
    set(CSHARP_OUTPUT_DIR ${OUTPUT_DIR})

    get_filename_component(PROJ_NAME ${CSHARP_PROJECT_PATH} NAME_WE)

    add_custom_target(Build_${PROJ_NAME}_Scripts ALL
            COMMAND dotnet publish ${CSHARP_PROJECT_PATH} -c Release -o ${CSHARP_OUTPUT_DIR}
            COMMENT "Building .NET project: ${CSHARP_PROJECT_PATH}"
            USES_TERMINAL
    )

    add_dependencies(${TARGET_NAME} Build_${PROJ_NAME}_Scripts)

endfunction()

function(add_dotnet_project_nobuild TARGET_NAME CSHARP_PROJECT_PATH OUTPUT_DIR)
    set(CSHARP_OUTPUT_DIR ${OUTPUT_DIR})

    get_filename_component(PROJ_NAME ${CSHARP_PROJECT_PATH} NAME_WE)

    add_custom_target(Build_${PROJ_NAME}_Scripts ALL
            COMMAND dotnet publish ${CSHARP_PROJECT_PATH} --no-build -c Release -o ${CSHARP_OUTPUT_DIR}
            COMMENT "Publishing .NET project (NoBuild): ${CSHARP_PROJECT_PATH}"
            USES_TERMINAL
    )

    add_dependencies(${TARGET_NAME} Build_${PROJ_NAME}_Scripts)
endfunction()
