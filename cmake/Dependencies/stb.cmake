
message(STATUS "== Including stb")

set(STB_DIRECTORY ${DEPENDENCIES_DIRECTORY}/stb)

set(STB_SOURCES
    ${STB_DIRECTORY}/stb_image.h
)

add_library(stb INTERFACE
    ${STB_SOURCES}
)

target_include_directories(stb INTERFACE
    ${STB_DIRECTORY}
)

##set_target_properties(stb PROPERTIES LINKER_LANGUAGE C)