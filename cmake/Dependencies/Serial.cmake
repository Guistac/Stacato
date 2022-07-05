
message(STATUS "== Including Serial")

set(SERIAL_DIRECTORY ${DEPENDENCIES_DIRECTORY}/Serial)

set(SERIAL_SOURCE_FILES
    ${SERIAL_DIRECTORY}/src/impl/list_ports/list_ports_osx.cc
    ${SERIAL_DIRECTORY}/src/impl/unix.cc
    ${SERIAL_DIRECTORY}/src/serial.cc
    ${SERIAL_DIRECTORY}/include/serial/serial.h
    ${SERIAL_DIRECTORY}/include/serial/v8stdint.h
    ${SERIAL_DIRECTORY}/include/serial/impl/unix.h
)

source_group(TREE ${SERIAL_DIRECTORY} FILES ${SERIAL_SOURCE_FILES})

add_library(serial ${SERIAL_SOURCE_FILES})

target_include_directories(serial PUBLIC
    ${SERIAL_DIRECTORY}
    ${SERIAL_DIRECTORY}/include
)
