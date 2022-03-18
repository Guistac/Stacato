
#declare the minimum required cmake version to build this file
cmake_minimum_required(VERSION 3.20.0)

#======= Workspace ========
project(Stacato VERSION 0.2 DESCRIPTION "Stage Control Automation Toolbox" LANGUAGES CXX)

#we are using C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32)
	set(STACATO_OS_NAME "Windows")
	set(STACATO_WIN32 true)
elseif(APPLE)
	set(STACATO_OS_NAME "MacOS")
	set(STACATO_MACOS true)
else()
	set(STACATO_OS_NAME "Unspecified")
endif()

# what is this again ??
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTACATO_DEBUG")

# Generate config.h file (will be in build binaries folder)
configure_file(src/core/config.h.in src/core/config.h)

# Compiler Configuration :
# Visual Studio
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	message("Compiler: Microsoft Visual Studio")
	message("Visual Studio : Enabling Edit and Continue")
	# Enable flags for Edit & Continue Feature
	set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
	set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO")
	set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Xcode / Apple Clang
elseif(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
	set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
	message("Compiler: Apple Clang")

# Other Compiler...
else()
	message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()


set(STACATO_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

# includes main build script
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Stacato.cmake)