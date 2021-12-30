
#----- CMakeLists.txt file for all dependencies -----



message("---GLFW---")
add_subdirectory(glfw)
set_target_properties(glfw PROPERTIES FOLDER "Dependencies/Graphics")

message("---NativeFileDialog---")
add_subdirectory(nativefiledialog-extended)
set_target_properties(nfd PROPERTIES FOLDER "Dependencies/IO")

message("---SpdLog---")
add_subdirectory(spdlog)
set_target_properties(spdlog PROPERTIES FOLDER "Dependencies/IO")

message("---TinyXML2---")
add_subdirectory(tinyxml2)
set_target_properties(tinyxml2 PROPERTIES FOLDER "Dependencies/IO")

message("---SOEM---")
add_subdirectory(soem)
set_target_properties(soem PROPERTIES FOLDER "Dependencies/Fieldbus")




#------ Magnum & Corrade Engine -------

if(WIN32)
    set(WITH_WGLCONTEXT ON)
	set(MSVC2019_COMPATIBILITY ON) ##avoid warning when including corrade
elseif(APPLE)
    set(WITH_CGLCONTEXT ON)
endif()

message("---Corrade---")
add_subdirectory(corrade EXCLUDE_FROM_ALL)

message("---Magnum---")
add_subdirectory(magnum EXCLUDE_FROM_ALL)

#set the directory for corrade and magnum package modules
set(Corrade_DIR corrade/modules)
set(Magnum_DIR magnum/modules)
find_package(Magnum REQUIRED)

if(CORRADE_TARGET_APPLE)
    find_package(Magnum REQUIRED CglContext)
elseif(CORRADE_TARGET_UNIX)
    find_package(Magnum REQUIRED GlxContext)
elseif(CORRADE_TARGET_WINDOWS)
    find_package(Magnum REQUIRED WglContext)
else()
    message(FATAL_ERROR "CORRADE: No context handler available on this platform")
endif()

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_library(MagnumLibs INTERFACE)

target_link_libraries(MagnumLibs INTERFACE
    Magnum::GL
    Magnum::GLContext
	Magnum::Magnum
	Magnum::Shaders
    Magnum::MeshTools
    Magnum::Primitives
)
    




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

target_include_directories(dearimgui PUBLIC
    "dearimgui/"
)

set_target_properties(dearimgui PROPERTIES FOLDER "Dependencies/Gui")

#================== IMPLOT =================

add_library(implot
	"implot/implot.cpp"
    "implot/implot.h"
    "implot/implot_demo.cpp"
    "implot/implot_internal.h"
    "implot/implot_items.cpp"
)

target_include_directories(implot PUBLIC
    "implot/"
    "dearimgui/"
    "glm/"
)

set_target_properties(implot PROPERTIES FOLDER "Dependencies/Gui")

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
    "imguinodeeditor"
	"dearimgui/"
	"glm/"
)

set_target_properties(imguinodeeditor PROPERTIES FOLDER "Dependencies/Gui")

#=================== GLAD ===================

add_library(glad
    "glad/include/glad/glad.h"
    "glad/include/KHR/khrplatform.h"
    "glad/src/glad.c"
)

target_include_directories(glad PUBLIC
   "glad/include/"
)

set_target_properties(glad PROPERTIES FOLDER "Dependencies/Graphics")

#=================== ASIO =====================

add_library(asio INTERFACE)

target_include_directories(asio INTERFACE
    "asio/asio/include/"
)

target_compile_definitions(asio INTERFACE
    ASIO_STANDALONE
)

#=================== TINYOSC =====================

add_library(tinyosc
	"tinyosc/tinyosc.c"
	"tinyosc/tinyosc.h"
	"tinyosc/OscMessage.cpp"
	"tinyosc/OscMessage.h"
)

target_include_directories(tinyosc PUBLIC "tinyosc/")

set_target_properties(tinyosc PROPERTIES FOLDER "Dependencies/Networking")

#================== GLM ====================

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
    "glm/"
)
