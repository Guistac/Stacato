
#----- CMakeLists.txt file for all dependencies -----

message("---GLFW---")
add_subdirectory(glfw)
message("---NativeFileDialog---")
add_subdirectory(nativefiledialog-extended)
message("---SpdLog---")
add_subdirectory(spdlog)
message("---TinyXML2---")
add_subdirectory(tinyxml2)



if(WIN32)
   message("---SOEM---")
   add_subdirectory(soem)
elseif(APPLE)

	add_library(SOEM PUBLIC
		soem/*.h
		soem/*.c
		soem/osal/osal.h
		
	)

endif()

#------ Magnum & Corrade Engine -------

if(WIN32)
    set(WITH_WGLCONTEXT ON)
endif()
if(APPLE)
    set(WITH_CGLCONTEXT ON)
endif()

if(WIN32)
	set(MSVC2019_COMPATIBILITY ON) ##avoid warning when including corrade
endIf()

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
    message(FATAL_ERROR "No context handler available on this platform")
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

#=================== GLAD ===================

add_library(glad
    "glad/include/glad/glad.h"
    "glad/include/KHR/khrplatform.h"
    "glad/src/glad.c"
)

target_include_directories(glad PUBLIC
   "glad/include/"
)

#=================== ASIO =====================

add_library(asio INTERFACE)

target_include_directories(asio INTERFACE
    "asio/asio/include/"
)

target_compile_definitions(asio INTERFACE
    ASIO_STANDALONE
)

#================== GLM ====================

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
    "glm/"
)
