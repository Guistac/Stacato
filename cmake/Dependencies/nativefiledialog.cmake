
message(STATUS "== Including Native File Dialog")

#SET(GLFW_BUILD_DOCS FALSE CACHE BOOL "" FORCE)

add_subdirectory(${DEPENDENCIES_DIRECTORY}/nativefiledialog-extended)