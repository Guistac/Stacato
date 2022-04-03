
include(${CMAKE_CURRENT_LIST_DIR}/ofxLua.cmake)

add_library(openFrameworksAddons INTERFACE)

target_link_libraries(openFrameworksAddons INTERFACE
    ofxLua
)