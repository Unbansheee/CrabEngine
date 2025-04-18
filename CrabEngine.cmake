function(target_copy_engine_resources Target)
    add_custom_command(
            TARGET ${Target} POST_BUILD
            COMMAND
            ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "'$<TARGET_FILE_DIR:${Target}'/Engine/resources">
            COMMENT
            "Copying resources from ${CMAKE_CURRENT_SOURCE_DIR}/resources to '$<TARGET_FILE_DIR:${Target}>'..."
    )
endfunction()