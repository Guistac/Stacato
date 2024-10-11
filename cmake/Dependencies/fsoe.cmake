
message(STATUS "== Including FSOE")

set(FSOE_DIRECTORY ${DEPENDENCIES_DIRECTORY}/fsoe/fsoe)

set(FSOE_SOURCE_FILES
    ${FSOE_DIRECTORY}/fsoeapp.h
    ${FSOE_DIRECTORY}/fsoeexport.h
    ${FSOE_DIRECTORY}/fsoemaster.h
    ${FSOE_DIRECTORY}/fsoeoptions.h
    ${FSOE_DIRECTORY}/fsoeslave.h
    ${FSOE_DIRECTORY}/fsoetypes.h
)

add_library(fsoe ${FSOE_SOURCE_FILES})

set_target_properties(fsoe PROPERTIES LINKER_LANGUAGE C)

target_include_directories(fsoe PUBLIC
    ${FSOE_DIRECTORY}
)

link_directories(${FSOE_SOURCE_FILES}/lib/linux)
