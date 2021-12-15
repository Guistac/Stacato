
#declare the minimum required cmake version to build this file
cmake_minimum_required(VERSION 3.20.0)

#======= Workspace ========
project(Stacato VERSION 0.1 DESCRIPTION "Stage Control Automation Toolbox" LANGUAGES CXX)

if(WIN32)
    set(STACATO_OS_NAME "Windows")
    set(STACATO_WIN32 true)
    set(CMAKE_CXX_STANDARD 17)
elseif(APPLE)
    set(STACATO_OS_NAME "MacOS")
    set(STACATO_MACOS true)
    set(CMAKE_CXX_STANDARD 17)
    option(STACATO_BELOW_MACOS_1_14 OFF)
    if(STACATO_BELOW_MACOS_1_14)
        set(CMAKE_CXX_STANDARD 14)
    endif()
else()
    set(STACATO_OS_NAME "Unspecified")
    set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTACATO_DEBUG")

# Generate config.h file (will be in build binaries folder)
configure_file(src/core/config.h.in src/core/config.h)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # For Visual Studio Enable flags for Edit & Continue Feature
    message("Compiler: Microsoft Visual Studio")
    message("Visual Studio : Enabling Edit and Continue")
    set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
    set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO")
    set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    # settings for xcode
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
    message("Compiler: Apple Clang")
else()
    message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

#======= Dependencies =========

#add all dependencies
add_subdirectory(dependencies)

##====== actual main project ========

#add directory containing all Project Source Files
file(GLOB_RECURSE SOURCE_FILE_LIST "src/*.cpp" "src/*.h")
list(APPEND SOURCE_FILE_LIST "src/Core/config.h.in")
list(APPEND SOURCE_FILE_LIST "${PROJECT_BINARY_DIR}/src/Core/config.h")
if(APPLE)
	list(APPEND SOURCE_FILE_LIST "${PROJECT_SOURCE_DIR}/src/Core/Info.plist")
endif()

# Main Executable with all source files in src/
if(WIN32)
	option(STACATO_WIN32_APPLICATION ON)
	if(STACATO_WIN32_APPLICATION)
		add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILE_LIST})
	else()
		add_executable(${PROJECT_NAME} ${SOURCE_FILE_LIST})
	endif()
elseif(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILE_LIST})
endif()

# use folders in ide
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# match on disk file structure for source files in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILE_LIST})

# Precompiled Headers
target_precompile_headers(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src/Core/pch.h
)

# Add the following include directories to the project
target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src		
	${PROJECT_SOURCE_DIR}/src/Core  # to include the precompiled header file
	${PROJECT_BINARY_DIR}/src/Core  # to include the custom config.h file
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
	asio

	#=== Saving & Loading Files ====
	nfd
	tinyxml2

	##=== Windowing & Gui ===
	glfw
	dearimgui
	imguinodeeditor
	implot

	#=== Math ===
	glm
)

option(STACATO_USE_MAGNUM_ENGINE OFF)

if(STACATO_USE_MAGNUM_ENGINE)
	target_link_libraries(${PROJECT_NAME} PRIVATE MagnumLibs)
	target_compile_definitions(${PROJECT_NAME} PRIVATE STACATO_USE_MAGNUM_ENGINE)
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE glad)
endif()


if(WIN32)
	target_link_directories(${PROJECT_NAME} PRIVATE
		#=== directory of the wpcap .lib binary included in soem repo ===
		#=== soem CMakeLists.txt handles linking wpcap.lib ===
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Lib/x64/
	)
	target_include_directories(${PROJECT_NAME} PRIVATE
		#=== include path for wpcap.lib headers ===
		${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Include
	)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WINDOWS _WIN32_WINNT=0x0601 WIN32)
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
	set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()


if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dir/Resources"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources
    )
	target_compile_definitions(${PROJECT_NAME} PRIVATE MACOS)
    set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dir/Resources")
    set_property(GLOBAL PROPERTY XCODE_SCHEME_EXECUTABLE ${PROJECT_NAME})
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/src/Core/Info.plist")
endif()
