
message(STATUS "== Including PsnCpp")

set(PSNCPP_DIRECTORY ${DEPENDENCIES_DIRECTORY}/psncpp)

add_library(psncpp INTERFACE)

target_include_directories(psncpp INTERFACE
    ${PSNCPP_DIRECTORY}/include
)