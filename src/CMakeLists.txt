
file(GLOB_RECURSE SOURCE_FILE_LIST "*.cpp" "*.h")

#add files to the project
add_executable(${PROJECT_NAME} ${SOURCE_FILE_LIST} )

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILE_LIST})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dir"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()