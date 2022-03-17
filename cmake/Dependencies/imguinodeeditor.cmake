
message(STATUS "== Including ImGuiNodeEditor")

set(IMGUINODEEDITOR_DIRECTORY ${DEPENDENCIES_DIRECTORY}/imguinodeeditor)

set(IMGUI_SOURCE_FILES
    ${IMGUINODEEDITOR_DIRECTORY}/
)

source_group(TREE ${IMGUINODEEDITOR_DIRECTORY} FILES ${IMGUI_SOURCE_FILES})

add_library(imguinodeeditor ${IMGUI_SOURCE_FILES})

target_include_directories(imguinodeeditor PUBLIC
    ${IMGUINODEEDITOR_DIRECTORY}/
)

target_link_libraries(imguinodeeditor PUBLIC
    imgui
)
