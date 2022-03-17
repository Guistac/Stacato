
message(STATUS "== Including TinyOSC")

#SET(GLFW_BUILD_DOCS FALSE CACHE BOOL "" FORCE)

set(TINYOSC_DIRECTORY ${DEPENDENCIES_DIRECTORY})

add_library(tinyosc
	${TINYOSC_DIRECTORY}/tinyosc.c
	${TINYOSC_DIRECTORY}/tinyosc.h
	${TINYOSC_DIRECTORY}/OscMessage.cpp
	${TINYOSC_DIRECTORY}/OscMessage.h
)

target_include_directories(tinyosc PUBLIC
    ${TINYOSC_DIRECTORY}
)

set_target_properties(tinyosc PROPERTIES FOLDER "Dependencies/Networking")