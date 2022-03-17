
message(STATUS "== Including TinyXML2")

SET(tinyxml2_BUILD_TESTING FALSE CACHE BOOL "" FORCE)

add_subdirectory(${DEPENDENCIES_DIRECTORY}/tinyxml2)