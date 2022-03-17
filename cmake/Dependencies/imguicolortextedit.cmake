
message(STATUS "== Including ImGuiColorTextEdit")

set(IMGUICOLORTEXTEDIT_DIRECTORY ${DEPENDENCIES_DIRECTORY}/imguicolortextedit)

set(IMGUICOLORTEXTEDIT_SOURCES
    ${IMGUICOLORTEXTEDIT_DIRECTORY}/TextEditor.cpp
    ${IMGUICOLORTEXTEDIT_DIRECTORY}/TextEditor.h
)

add_library(imguicolortextedit STATIC
    ${IMGUICOLORTEXTEDIT_SOURCES}
)

target_include_directories(imguicolortextedit PUBLIC
    ${IMGUICOLORTEXTEDIT_DIRECTORY}/
    ${DEPENDENCIES_DIRECTORY}/dearimgui
)