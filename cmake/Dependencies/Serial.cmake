
##WARNING: we are not currently using the shipped Cmake script
##there could be issues with the library due to this

message(STATUS "== Including Serial")

set(SERIAL_DIRECTORY ${DEPENDENCIES_DIRECTORY}/Serial)



set(SERIAL_SOURCE_FILES
    ${SERIAL_DIRECTORY}/src/serial.cc
    ${SERIAL_DIRECTORY}/include/serial/serial.h
    ${SERIAL_DIRECTORY}/include/serial/v8stdint.h
)

if(APPLE)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/src/impl/unix.cc)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/src/impl/list_ports/list_ports_osx.cc)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/include/serial/impl/unix.h)
elseif(UNIX)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/src/impl/unix.cc)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/src/impl/list_ports/list_ports_linux.cc)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/include/serial/impl/unix.h)
else()
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/src/impl/win.cc)
    list(APPEND SERIAL_SOURCE_FILES ${SERIAL_DIRECTORY}/src/impl/list_ports/list_ports_win.cc)
endif()




source_group(TREE ${SERIAL_DIRECTORY} FILES ${SERIAL_SOURCE_FILES})

add_library(serial ${SERIAL_SOURCE_FILES})

target_include_directories(serial PUBLIC
    ${SERIAL_DIRECTORY}
    ${SERIAL_DIRECTORY}/include
)
