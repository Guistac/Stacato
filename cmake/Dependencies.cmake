set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)

include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/dearimgui.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/imguicolortextedit.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/nativefiledialog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/soem.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/spdlog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/tinyxml2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/imguinodeeditor.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/implot.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/asio.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/tinyosc.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies/openFrameworks.cmake)

set(STACATO_DEPENDENCIES
    glfw
    dearimgui
    imguicolortextedit
    openFrameworks
    nfd
    soem
    spdlog
    tinyxml2
    imguinodeeditor
    implot
    asio
    glm
    tinyosc
)