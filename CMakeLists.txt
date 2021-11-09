#declare the minimum required cmake version to build this file
cmake_minimum_required(VERSION 3.20.0)

#defines the workspace
project(Stacato VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CSS_STANDARD_REQUIRED True)

#======= Dependencies =========

#add other CMakeLists.txt files located in these subdirectories
add_subdirectory(dependencies/dearimgui)
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/imguinodeeditor)
add_subdirectory(dependencies/implot)
add_subdirectory(dependencies/nativefiledialog-extended)
add_subdirectory(dependencies/soem)
add_subdirectory(dependencies/spdlog)
add_subdirectory(dependencies/tinyxml2)

##====== actual main project ========

#add directory containing all Project Source Files
#define the executable target
##add_subdirectory(src)

file(GLOB_RECURSE SOURCE_FILE_LIST "src/*.cpp" "src/*.h")

#add files to the project
add_executable(${PROJECT_NAME} ${SOURCE_FILE_LIST} )

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILE_LIST})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

target_compile_definitions(${PROJECT_NAME} PUBLIC
	ASIO_STANDALONE
	_CRT_SECURE_NO_WARNINGS
)

#writes the project version number to config.h (will be in build binaries folder)
configure_file(src/core/config.h.in src/core/config.h)

#link the project with librairies we just added
target_link_libraries(${PROJECT_NAME} 
	glfw
	spdlog
	soem
	nfd
	tinyxml2
	dearimgui
	glad
	imguinodeeditor
	implot
)

#add the following include directories to the project
target_include_directories(${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/dependencies/soem/soem/
	${PROJECT_SOURCE_DIR}/dependencies/soem/osal/
	${PROJECT_SOURCE_DIR}/dependencies/asio/asio/include/

	${PROJECT_SOURCE_DIR}/dependencies/glm/
	${PROJECT_SOURCE_DIR}/dependencies/spdlog/include/
	${PROJECT_SOURCE_DIR}/dependencies/nativefiledialog-extended/src/include/
	${PROJECT_SOURCE_DIR}/dependencies/tinyxml2/
	
	${PROJECT_SOURCE_DIR}/dependencies/glad/include
	${PROJECT_SOURCE_DIR}/dependencies/glfw/include/
	${PROJECT_SOURCE_DIR}/dependencies/dearimgui/
	${PROJECT_SOURCE_DIR}/dependencies/implot/
	${PROJECT_SOURCE_DIR}/dependencies/imguinodeeditor/

	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/Core

	${PROJECT_BINARY_DIR}/src/Core
)

target_precompile_headers(${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/src/Core/pch.h
)

if(WIN32)
	target_link_directories(${PROJECT_NAME} PUBLIC
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Lib/x64/
	)
	target_link_libraries(${PROJECT_NAME} 
		wpcap
	)
	target_include_directories(${PROJECT_NAME} PUBLIC
		${PROJECT_SOURCE_DIR}/dependencies/soem/osal/win32
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Include
	)
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		_WINDOWS
		_WIN32_WINNT=0x0601
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
endif()

if(APPLE)
	##target_link_directories(${PROJECT_NAME} PUBLIC)
	target_link_libraries(${PROJECT_NAME} 
		Cocoa.framework
        IOKit.framework
	)
	target_include_directories(${PROJECT_NAME} PUBLIC
		${PROJECT_SOURCE_DIR}/dependencies/soem/osal/macosx
        ${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/macosx
	)
	target_compile_definitions("${PROJECT_NAME}" PUBLIC
		_WINDOWS
		_WIN32_WINNT=0x0601
	)
endif()

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Stacato)
