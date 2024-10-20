
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

add_library(fsoe_demo ${FSOE_SOURCE_FILES})

set_target_properties(fsoe_demo PROPERTIES LINKER_LANGUAGE C)

target_include_directories(fsoe_demo PUBLIC ${FSOE_DIRECTORY})

target_link_libraries(fsoe_demo PUBLIC ${FSOE_DIRECTORY}/lib/linux/libfsoe.a)
