
set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)

include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dearimgui.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/imguicolortextedit.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/openFrameworks.cmake)

set(DEPENDENCY_LINKER_LIST
    glfw
    dearimgui
    imguicolortextedit
    openFrameworks
)