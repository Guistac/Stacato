
message(STATUS "== Including GLM")

#SET(GLFW_BUILD_DOCS FALSE CACHE BOOL "" FORCE)

set(GLM_DIRECTORY ${DEPENDENCIES_DIRECTORY}/glm)

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
    ${GLM_DIRECTORY}
)