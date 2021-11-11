
#declare the minimum required cmake version to build this file
cmake_minimum_required(VERSION 3.20.0)

#defines the workspace
project(Stacato VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CSS_STANDARD_REQUIRED True)

#writes the project version number to config.h (will be in build binaries folder)
configure_file(src/core/config.h.in src/core/config.h)



#======= Dependencies =========

#add all dependencies
add_subdirectory(dependencies)



##====== actual main project ========

#add directory containing all Project Source Files
file(GLOB_RECURSE SOURCE_FILE_LIST "src/*.cpp" "src/*.h")

# Main Executable with all source files in src/
if(WIN32)
    add_executable(${PROJECT_NAME} ${SOURCE_FILE_LIST})
elseif(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILE_LIST})
endif()

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILE_LIST})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


target_precompile_headers(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src/Core/pch.h
)

#add the following include directories to the project
target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src		
	${PROJECT_SOURCE_DIR}/src/Core #=== to include the precompiled header file ===
	${PROJECT_BINARY_DIR}/src/Core #=== to include the custom config.h file ===
)


target_compile_definitions(${PROJECT_NAME} PRIVATE
	_CRT_SECURE_NO_WARNINGS
)


#link the project with librairies we just added
target_link_libraries(${PROJECT_NAME} PRIVATE
	#=== Logging ===
	spdlog

	#=== Network ===
	soem
	glm

	#=== Saving & Loading Files ====
	nfd
	tinyxml2

	##=== Windowing & Gui ===
	glfw
	#glad
	dearimgui
	imguinodeeditor
	implot

	#=== Math ===
	asio

	#=== Magnum Librairies ===
	MagnumLibs
)


if(WIN32)
	target_link_directories(${PROJECT_NAME} PRIVATE
		#=== directory of the wpcap .lib binary included in soem repo
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Lib/x64/
	)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		#=== links actual wpcap .lib binary included in soem repo
		wpcap
	)
	target_include_directories(${PROJECT_NAME} PRIVATE
		${PROJECT_SOURCE_DIR}/dependencies/soem/osal/win32
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Include
	)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		_WINDOWS
		_WIN32_WINNT=0x0601
        WIN32
	)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dir"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
	set_target_properties(${PROJECT_NAME}
		#sets debug working direction for visual studio solution
		PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dir"
	)
	# Make the executable a default target to build & run in Visual Studio
	set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Stacato)
endif()


if(APPLE)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		#Cocoa.framework
        #IOKit.framework
	)
	target_include_directories(${PROJECT_NAME} PRIVATE
		${PROJECT_SOURCE_DIR}/dependencies/soem/osal/macosx
        ${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/macosx
	)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dir/Resources"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources
    )
	target_compile_definitions("${PROJECT_NAME}" PRIVATE
		MACOS
	)
endif()
