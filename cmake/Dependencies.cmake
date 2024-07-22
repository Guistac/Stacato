set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)

# Windowing
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/glfw.cmake)

# Math
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/glm.cmake)

# Graphical User Interface
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/dearimgui.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/imguinodeeditor.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/implot.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/imguicolortextedit.cmake)

# Networking
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/soem.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/asio.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/tinyosc.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/psncpp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/artnode.cmake)

# I/O
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/nativefiledialog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/spdlog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/tinyxml2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/stb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/Serial.cmake)

# Scripting
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/lua.cmake)

set(STACATO_DEPENDENCIES
# Windowing
    glfw
# Math
    glm
# Graphical User Interface
    dearimgui
    imguicolortextedit
    imguinodeeditor
    implot
# Networking
    soem
    asio
    tinyosc
    psncpp
    artnode
# I/O
    nfd
    spdlog
    tinyxml2
    stb
    serial
# Scripting
    lua_lib
)