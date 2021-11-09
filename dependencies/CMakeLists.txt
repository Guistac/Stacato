
#----- CMakeLists.txt file for all dependencies -----

add_subdirectory(glfw)
add_subdirectory(nativefiledialog-extended)
add_subdirectory(soem)
add_subdirectory(spdlog)
add_subdirectory(tinyxml2)

#----- Below are build infos for dependencies that don't ship a CMakeLists.txt file -----

#============= DEAR IMGUI ==============

add_library(dearimgui
    "dearimgui/imconfig.h"
    "dearimgui/imgui.h"
    "dearimgui/imgui.cpp"
    "dearimgui/imgui_draw.cpp"
    "dearimgui/imgui_internal.h"
    "dearimgui/imgui_widgets.cpp"
    "dearimgui/imgui_tables.cpp"
    "dearimgui/imstb_rectpack.h"
    "dearimgui/imstb_textedit.h"
    "dearimgui/imstb_truetype.h"
    "dearimgui/imgui_demo.cpp"
)

#================== IMPLOT =================

add_library(implot
	"implot/implot.cpp"
    "implot/implot.h"
    "implot/implot_demo.cpp"
    "implot/implot_internal.h"
    "implot/implot_items.cpp"
)

target_include_directories(implot PUBLIC
    "dearimgui/"
    "glm/"
)

#=============== IMGUI NODE EDITOR ===============

add_library(imguinodeeditor
	"imguinodeeditor/crude_json.cpp"
	"imguinodeeditor/crude_json.h"
	"imguinodeeditor/imgui_bezier_math.h"
	"imguinodeeditor/imgui_bezier_math.inl"
	"imguinodeeditor/imgui_canvas.cpp"
	"imguinodeeditor/imgui_canvas.h"
	"imguinodeeditor/imgui_extra_math.h"
	"imguinodeeditor/imgui_extra_math.inl"
	"imguinodeeditor/imgui_node_editor.cpp"
	"imguinodeeditor/imgui_node_editor.h"
	"imguinodeeditor/imgui_node_editor_api.cpp"
	"imguinodeeditor/imgui_node_editor_internal.h"
	"imguinodeeditor/imgui_node_editor_internal.inl"
)


target_include_directories(imguinodeeditor PUBLIC
	"dearimgui/"
	"glm/"
)

#=================== GLAD ===================

add_library(glad
    "glad/include/glad/glad.h"
    "glad/include/KHR/khrplatform.h"
    "glad/src/glad.c"
)

target_include_directories(glad PUBLIC
   "glad/include/"
)