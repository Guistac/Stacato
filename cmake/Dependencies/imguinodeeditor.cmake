
message(STATUS "== Including ImGuiNodeEditor")

set(IMGUINODEEDITOR_DIRECTORY ${DEPENDENCIES_DIRECTORY}/imguinodeeditor)

set(IMGUINODEEDITOR_SOURCE_FILES
    ${IMGUINODEEDITOR_DIRECTORY}/crude_json.cpp
	${IMGUINODEEDITOR_DIRECTORY}/crude_json.h
	${IMGUINODEEDITOR_DIRECTORY}/imgui_bezier_math.h
	${IMGUINODEEDITOR_DIRECTORY}/imgui_bezier_math.inl
	${IMGUINODEEDITOR_DIRECTORY}/imgui_canvas.cpp
	${IMGUINODEEDITOR_DIRECTORY}/imgui_canvas.h
	${IMGUINODEEDITOR_DIRECTORY}/imgui_extra_math.h
	${IMGUINODEEDITOR_DIRECTORY}/imgui_extra_math.inl
	${IMGUINODEEDITOR_DIRECTORY}/imgui_node_editor.cpp
	${IMGUINODEEDITOR_DIRECTORY}/imgui_node_editor.h
	${IMGUINODEEDITOR_DIRECTORY}/imgui_node_editor_api.cpp
	${IMGUINODEEDITOR_DIRECTORY}/imgui_node_editor_internal.h
	${IMGUINODEEDITOR_DIRECTORY}/imgui_node_editor_internal.inl
)

source_group(TREE ${IMGUINODEEDITOR_DIRECTORY} FILES ${IMGUINODEEDITOR_SOURCE_FILES})

add_library(imguinodeeditor ${IMGUINODEEDITOR_SOURCE_FILES})

target_include_directories(imguinodeeditor PUBLIC
    ${IMGUINODEEDITOR_DIRECTORY}/
	${DEPENDENCIES_DIRECTORY}/dearimgui
)

target_link_libraries(imguinodeeditor PUBLIC
    imgui
)
