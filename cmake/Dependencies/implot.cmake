
message(STATUS "== Including ImPlot")

set(IMPLOT_DIRECTORY ${DEPENDENCIES_DIRECTORY}/implot)

set(IMPLOT_SOURCE_FILES
    ${IMPLOT_DIRECTORY}/implot.cpp
    ${IMPLOT_DIRECTORY}/implot.h
    ${IMPLOT_DIRECTORY}/implot_demo.cpp
    ${IMPLOT_DIRECTORY}/implot_internal.h
    ${IMPLOT_DIRECTORY}/implot_items.cpp
)

source_group(TREE ${IMPLOT_DIRECTORY} FILES ${IMPLOT_SOURCE_FILES})

add_library(implot ${IMPLOT_SOURCE_FILES})

target_include_directories(implot PUBLIC
    ${IMPLOT_DIRECTORY}/
)

target_link_libraries(implot PUBLIC
    imgui
)
