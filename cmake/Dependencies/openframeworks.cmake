
message(STATUS "== Including OpenFrameworks")

set(OF_DIRECTORY ${DEPENDENCIES_DIRECTORY}/openFrameworks)
set(OF_LIBS_DIRECTORY ${OF_DIRECTORY}/libs)
set(OF_CORE_DIRECTORY ${OF_LIBS_DIRECTORY}/openFrameworks)

#=== List all OpenFrameworks Precompiled Libraries
# we need to check if they all exist
# else we need to download them using the provided script
set(OF_CORE_LIBRARIES
    ${OF_LIBS_DIRECTORY}/freetype/lib/osx/freetype.a
    ${OF_LIBS_DIRECTORY}/FreeImage/lib/osx/freeimage.a
    ${OF_LIBS_DIRECTORY}/fmod/lib/osx/libfmod.dylib
    ${OF_LIBS_DIRECTORY}/glew/lib/osx/glew.a
    ${OF_LIBS_DIRECTORY}/tess2/lib/osx/tess2.a
    ${OF_LIBS_DIRECTORY}/cairo/lib/osx/cairo-script-interpreter.a
    ${OF_LIBS_DIRECTORY}/cairo/lib/osx/cairo.a
    ${OF_LIBS_DIRECTORY}/cairo/lib/osx/pixman-1.a
    ${OF_LIBS_DIRECTORY}/rtAudio/lib/osx/rtaudio.a
    #${OF_LIBS_DIRECTORY}/glfw/lib/osx/glfw3.a                      # not used since we build glfw externally
    ${OF_LIBS_DIRECTORY}/curl/lib/osx/curl.a
    ${OF_LIBS_DIRECTORY}/uriparser/lib/osx/uriparser.a
    #${OF_LIBS_DIRECTORY}/boost/lib/osx/boost_system.a              # not used since we have C++20 for std::filesystem
    #${OF_LIBS_DIRECTORY}/boost/lib/osx/boost_filesystem.a          # not used since we have C++20 for std::filesystem
    ${OF_LIBS_DIRECTORY}/pugixml/lib/osx/pugixml.a
)

message(STATUS "Checking for precompiled dependencies...")
set(FOUND_OF_LIBRARIES TRUE)
foreach(FILE IN LISTS OF_CORE_LIBRARIES)
    if(NOT EXISTS ${FILE})
        message("Could not find Library : ${FILE}")
        set(FOUND_OF_LIBRARIES FALSE)
    endif()
endforeach()

if(NOT FOUND_OF_LIBRARIES)
    message("============================ ERROR ===========================\n"
            "Precompiled Openframeworks Dependencies were Not Found !\n"
            "Please run the following script to download them:\n"
            "${OF_DIRECTORY}/scripts/<YOUR_PLATFORM>/download_libs.sh\n"
            "============================ ERROR ===========================")
    message(FATAL_ERROR "=== could not load openframeworks dependencies ===")
else()
    message(STATUS "Found all precompiled dependencies ! ")
endif()


#=== List all OpenFrameworks Core Source & Header files
set(OF_CORE_SOURCE_FILES
    ${OF_CORE_DIRECTORY}/3d/of3dUtils.cpp
    ${OF_CORE_DIRECTORY}/3d/of3dUtils.h
    ${OF_CORE_DIRECTORY}/3d/ofCamera.cpp
    ${OF_CORE_DIRECTORY}/3d/ofCamera.h
    ${OF_CORE_DIRECTORY}/3d/ofEasyCam.cpp
    ${OF_CORE_DIRECTORY}/3d/ofEasyCam.h
    ${OF_CORE_DIRECTORY}/3d/ofMesh.inl
    ${OF_CORE_DIRECTORY}/3d/ofMesh.h
    ${OF_CORE_DIRECTORY}/3d/ofNode.cpp
    ${OF_CORE_DIRECTORY}/3d/ofNode.h
    ${OF_CORE_DIRECTORY}/3d/of3dPrimitives.cpp
    ${OF_CORE_DIRECTORY}/3d/of3dPrimitives.h

    ${OF_CORE_DIRECTORY}/app/ofWindowSettings.h
    ${OF_CORE_DIRECTORY}/app/ofMainLoop.cpp
    ${OF_CORE_DIRECTORY}/app/ofMainLoop.h
    ${OF_CORE_DIRECTORY}/app/ofAppNoWindow.cpp
    ${OF_CORE_DIRECTORY}/app/ofAppNoWindow.h
    ${OF_CORE_DIRECTORY}/app/ofAppGLFWWindow.cpp
    ${OF_CORE_DIRECTORY}/app/ofAppGLFWWindow.h
    ${OF_CORE_DIRECTORY}/app/ofAppBaseWindow.h
    ${OF_CORE_DIRECTORY}/app/ofAppRunner.cpp
    ${OF_CORE_DIRECTORY}/app/ofAppRunner.h
    ${OF_CORE_DIRECTORY}/app/ofBaseApp.cpp
    ${OF_CORE_DIRECTORY}/app/ofBaseApp.h

    ${OF_CORE_DIRECTORY}/communication/ofArduino.cpp
    ${OF_CORE_DIRECTORY}/communication/ofArduino.h
    ${OF_CORE_DIRECTORY}/communication/ofSerial.cpp
    ${OF_CORE_DIRECTORY}/communication/ofSerial.h

    ${OF_CORE_DIRECTORY}/events/ofEvent.h
    ${OF_CORE_DIRECTORY}/events/ofEvents.cpp
    ${OF_CORE_DIRECTORY}/events/ofEvents.h
    ${OF_CORE_DIRECTORY}/events/ofEventUtils.h

    ${OF_CORE_DIRECTORY}/gl/ofGLBaseTypes.h
    ${OF_CORE_DIRECTORY}/gl/ofBufferObject.cpp
    ${OF_CORE_DIRECTORY}/gl/ofBufferObject.h
    ${OF_CORE_DIRECTORY}/gl/ofGLProgrammableRenderer.cpp
    ${OF_CORE_DIRECTORY}/gl/ofGLProgrammableRenderer.h
    ${OF_CORE_DIRECTORY}/gl/ofFbo.cpp
    ${OF_CORE_DIRECTORY}/gl/ofFbo.h
    ${OF_CORE_DIRECTORY}/gl/ofGLRenderer.cpp
    ${OF_CORE_DIRECTORY}/gl/ofGLRenderer.h
    ${OF_CORE_DIRECTORY}/gl/ofGLUtils.cpp
    ${OF_CORE_DIRECTORY}/gl/ofGLUtils.h
    ${OF_CORE_DIRECTORY}/gl/ofLight.cpp
    ${OF_CORE_DIRECTORY}/gl/ofLight.h
    ${OF_CORE_DIRECTORY}/gl/ofMaterial.cpp
    ${OF_CORE_DIRECTORY}/gl/ofMaterial.h
    ${OF_CORE_DIRECTORY}/gl/ofShader.cpp
    ${OF_CORE_DIRECTORY}/gl/ofShader.h
    ${OF_CORE_DIRECTORY}/gl/ofTexture.cpp
    ${OF_CORE_DIRECTORY}/gl/ofTexture.h
    ${OF_CORE_DIRECTORY}/gl/ofVbo.cpp
    ${OF_CORE_DIRECTORY}/gl/ofVbo.h
    ${OF_CORE_DIRECTORY}/gl/ofVboMesh.cpp
    ${OF_CORE_DIRECTORY}/gl/ofVboMesh.h

    ${OF_CORE_DIRECTORY}/graphics/ofGraphicsBaseTypes.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofGraphicsBaseTypes.h
    ${OF_CORE_DIRECTORY}/graphics/ofPath.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofPath.h
    ${OF_CORE_DIRECTORY}/graphics/ofPolyline.inl
    ${OF_CORE_DIRECTORY}/graphics/ofPolyline.h
    ${OF_CORE_DIRECTORY}/graphics/ofRendererCollection.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofRendererCollection.h
    ${OF_CORE_DIRECTORY}/graphics/ofCairoRenderer.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofCairoRenderer.h
    ${OF_CORE_DIRECTORY}/graphics/ofBitmapFont.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofBitmapFont.h
    ${OF_CORE_DIRECTORY}/graphics/ofGraphics.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofGraphics.h
    ${OF_CORE_DIRECTORY}/graphics/of3dGraphics.cpp
    ${OF_CORE_DIRECTORY}/graphics/of3dGraphics.h
    ${OF_CORE_DIRECTORY}/graphics/ofImage.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofImage.h
    ${OF_CORE_DIRECTORY}/graphics/ofPixels.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofPixels.h
    ${OF_CORE_DIRECTORY}/graphics/ofTessellator.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofTessellator.h
    ${OF_CORE_DIRECTORY}/graphics/ofTrueTypeFont.cpp
    ${OF_CORE_DIRECTORY}/graphics/ofTrueTypeFont.h

    ${OF_CORE_DIRECTORY}/sound/ofSoundBaseTypes.cpp
    ${OF_CORE_DIRECTORY}/sound/ofSoundBaseTypes.h
    ${OF_CORE_DIRECTORY}/sound/ofFmodSoundPlayer.cpp
    ${OF_CORE_DIRECTORY}/sound/ofFmodSoundPlayer.h
    ${OF_CORE_DIRECTORY}/sound/ofOpenALSoundPlayer.cpp
    ${OF_CORE_DIRECTORY}/sound/ofOpenALSoundPlayer.h
    ${OF_CORE_DIRECTORY}/sound/ofRtAudioSoundStream.cpp
    ${OF_CORE_DIRECTORY}/sound/ofRtAudioSoundStream.h
    ${OF_CORE_DIRECTORY}/sound/ofSoundBuffer.cpp
    ${OF_CORE_DIRECTORY}/sound/ofSoundBuffer.h
    ${OF_CORE_DIRECTORY}/sound/ofSoundPlayer.cpp
    ${OF_CORE_DIRECTORY}/sound/ofSoundPlayer.h
    ${OF_CORE_DIRECTORY}/sound/ofSoundStream.cpp
    ${OF_CORE_DIRECTORY}/sound/ofSoundStream.h
    ${OF_CORE_DIRECTORY}/sound/ofSoundUtils.h

    ${OF_CORE_DIRECTORY}/math/ofMathConstants.h
    ${OF_CORE_DIRECTORY}/math/ofMath.cpp
    ${OF_CORE_DIRECTORY}/math/ofMath.h
    ${OF_CORE_DIRECTORY}/math/ofMatrix3x3.cpp
    ${OF_CORE_DIRECTORY}/math/ofMatrix3x3.h
    ${OF_CORE_DIRECTORY}/math/ofMatrix4x4.cpp
    ${OF_CORE_DIRECTORY}/math/ofMatrix4x4.h
    ${OF_CORE_DIRECTORY}/math/ofQuaternion.cpp
    ${OF_CORE_DIRECTORY}/math/ofQuaternion.h
    ${OF_CORE_DIRECTORY}/math/ofVec2f.cpp
    ${OF_CORE_DIRECTORY}/math/ofVec2f.h
    ${OF_CORE_DIRECTORY}/math/ofVec3f.h
    ${OF_CORE_DIRECTORY}/math/ofVec4f.cpp
    ${OF_CORE_DIRECTORY}/math/ofVec4f.h
    ${OF_CORE_DIRECTORY}/math/ofVectorMath.h

    ${OF_CORE_DIRECTORY}/types/ofParameter.cpp
    ${OF_CORE_DIRECTORY}/types/ofParameter.h
    ${OF_CORE_DIRECTORY}/types/ofParameterGroup.cpp
    ${OF_CORE_DIRECTORY}/types/ofParameterGroup.h
    ${OF_CORE_DIRECTORY}/types/ofBaseTypes.cpp
    ${OF_CORE_DIRECTORY}/types/ofBaseTypes.h
    ${OF_CORE_DIRECTORY}/types/ofColor.cpp
    ${OF_CORE_DIRECTORY}/types/ofColor.h
    ${OF_CORE_DIRECTORY}/types/ofPoint.h
    ${OF_CORE_DIRECTORY}/types/ofRectangle.cpp
    ${OF_CORE_DIRECTORY}/types/ofRectangle.h
    ${OF_CORE_DIRECTORY}/types/ofTypes.h

    ${OF_CORE_DIRECTORY}/utils/ofFpsCounter.cpp
    ${OF_CORE_DIRECTORY}/utils/ofFpsCounter.h
    ${OF_CORE_DIRECTORY}/utils/ofTimer.cpp
    ${OF_CORE_DIRECTORY}/utils/ofTimer.h
    ${OF_CORE_DIRECTORY}/utils/ofXml.cpp
    ${OF_CORE_DIRECTORY}/utils/ofXml.h
    ${OF_CORE_DIRECTORY}/utils/ofMatrixStack.cpp
    ${OF_CORE_DIRECTORY}/utils/ofMatrixStack.h
    ${OF_CORE_DIRECTORY}/utils/ofConstants.h
    ${OF_CORE_DIRECTORY}/utils/ofFileUtils.cpp
    ${OF_CORE_DIRECTORY}/utils/ofFileUtils.h
    ${OF_CORE_DIRECTORY}/utils/ofLog.cpp
    ${OF_CORE_DIRECTORY}/utils/ofLog.h
    ${OF_CORE_DIRECTORY}/utils/ofNoise.h
    ${OF_CORE_DIRECTORY}/utils/ofSystemUtils.cpp
    ${OF_CORE_DIRECTORY}/utils/ofSystemUtils.h
    ${OF_CORE_DIRECTORY}/utils/ofThread.cpp
    ${OF_CORE_DIRECTORY}/utils/ofThread.h
    ${OF_CORE_DIRECTORY}/utils/ofURLFileLoader.cpp
    ${OF_CORE_DIRECTORY}/utils/ofURLFileLoader.h
    ${OF_CORE_DIRECTORY}/utils/ofUtils.cpp
    ${OF_CORE_DIRECTORY}/utils/ofUtils.h

    ${OF_CORE_DIRECTORY}/video/ofVideoBaseTypes.h
    ${OF_CORE_DIRECTORY}/video/ofVideoGrabber.cpp
    ${OF_CORE_DIRECTORY}/video/ofVideoGrabber.h
    ${OF_CORE_DIRECTORY}/video/ofVideoPlayer.cpp
    ${OF_CORE_DIRECTORY}/video/ofVideoPlayer.h
    ${OF_CORE_DIRECTORY}/video/ofQuickTimeGrabber.cpp
    ${OF_CORE_DIRECTORY}/video/ofQuickTimeGrabber.h
    ${OF_CORE_DIRECTORY}/video/ofQuickTimePlayer.cpp
    ${OF_CORE_DIRECTORY}/video/ofQuickTimePlayer.h
    ${OF_CORE_DIRECTORY}/video/ofQtUtils.cpp
    ${OF_CORE_DIRECTORY}/video/ofQtUtils.h
    ${OF_CORE_DIRECTORY}/video/ofQTKitPlayer.mm
    ${OF_CORE_DIRECTORY}/video/ofQTKitPlayer.h
    ${OF_CORE_DIRECTORY}/video/ofQtKitMovieRenderer.m
    ${OF_CORE_DIRECTORY}/video/ofQtKitMovieRenderer.h
    ${OF_CORE_DIRECTORY}/video/ofAVFoundationGrabber.mm
    ${OF_CORE_DIRECTORY}/video/ofAVFoundationGrabber.h
    ${OF_CORE_DIRECTORY}/video/ofAVFoundationVideoPlayer.m
    ${OF_CORE_DIRECTORY}/video/ofAVFoundationVideoPlayer.h
    ${OF_CORE_DIRECTORY}/video/ofAVFoundationPlayer.mm
    ${OF_CORE_DIRECTORY}/video/ofAVFoundationPlayer.h
)

#=== Structure files into folders in IDE starting at the libs directory
source_group(TREE ${OF_CORE_DIRECTORY} FILES ${OF_CORE_SOURCE_FILES})



#=== List all OpenFrameworks Core Include Paths
set(OF_CORE_INCLUDE_PATHS
    ${OF_CORE_DIRECTORY}
    ${OF_CORE_DIRECTORY}/3d
    ${OF_CORE_DIRECTORY}/app
    ${OF_CORE_DIRECTORY}/communication
    ${OF_CORE_DIRECTORY}/events
    ${OF_CORE_DIRECTORY}/gl
    ${OF_CORE_DIRECTORY}/graphics
    ${OF_CORE_DIRECTORY}/math
    ${OF_CORE_DIRECTORY}/sound
    ${OF_CORE_DIRECTORY}/types
    ${OF_CORE_DIRECTORY}/utils
    ${OF_CORE_DIRECTORY}/video
)

#=== List all OpenFrameworks Library Dependecy Paths
set(OF_LIB_INCLUDE_PATHS
    ${OF_LIBS_DIRECTORY}/freetype/include
    ${OF_LIBS_DIRECTORY}/freetype/include/freetype2
    ${OF_LIBS_DIRECTORY}/FreeImage/include
    ${OF_LIBS_DIRECTORY}/fmod/include
    ${OF_LIBS_DIRECTORY}/glew/include
    ${OF_LIBS_DIRECTORY}/tess2/include
    ${OF_LIBS_DIRECTORY}/cairo/include/cairo
    ${OF_LIBS_DIRECTORY}/rtAudio/include
    #${OF_LIBS_DIRECTORY}/glfw/include      # not used since we build glfw externally
    ${OF_LIBS_DIRECTORY}/curl/include
    ${OF_LIBS_DIRECTORY}/uriparser/include
    #${OF_LIBS_DIRECTORY}/boost/include     # not used since we have C++20 for std::filesystem
    ${OF_LIBS_DIRECTORY}/pugixml/include
    ${OF_LIBS_DIRECTORY}/utf8/include
    ${OF_LIBS_DIRECTORY}/json/include
    ${OF_LIBS_DIRECTORY}/glm/include
)


#=== Find all OpenFrameworks MacOS Framework dependencies
find_library(OF_FRAMEWORK_ACCELERATE            Accelerate)
find_library(OF_FRAMEWORK_AGL                   AGL)
find_library(OF_FRAMEWORK_APPKIT                AppKit)
find_library(OF_FRAMEWORK_APPLICATIONSERVICES   ApplicationServices)
find_library(OF_FRAMEWORK_AUDIOTOOLBOX          AudioToolbox)
find_library(OF_FRAMEWORK_AVFOUNDATION          AVFoundation)
#find_library(OF_FRAMEWORK_COCOA                 Cocoa) # not used since we build glfw externally
find_library(OF_FRAMEWORK_COREAUDIO             CoreAudio)
find_library(OF_FRAMEWORK_COREFOUNDATION        CoreFoundation)
find_library(OF_FRAMEWORK_COREMEDIA             CoreMedia)
find_library(OF_FRAMEWORK_CORESERVICES          CoreServices)
find_library(OF_FRAMEWORK_COREVIDEO             CoreVideo)
find_library(OF_FRAMEWORK_IOKIT                 IOKit)
find_library(OF_FRAMEWORK_OPENGL                OpenGL)
find_library(OF_FRAMEWORK_QUARTZCORE            QuartzCore)
find_library(OF_FRAMEWORK_QTKIT                 QTKit)
find_library(OF_FRAMEWORK_SECURITY              Security)

#=== List all OpenFrameworks MacOS Framework dependencies
set(OF_CORE_FRAMEWORKS
    ${OF_FRAMEWORK_ACCELERATE}          
    ${OF_FRAMEWORK_AGL}                 
    ${OF_FRAMEWORK_APPKIT}              
    ${OF_FRAMEWORK_APPLICATIONSERVICES} 
    ${OF_FRAMEWORK_AUDIOTOOLBOX}        
    ${OF_FRAMEWORK_AVFOUNDATION}        
    #${OF_FRAMEWORK_COCOA} # not used since we build glfw externally         
    ${OF_FRAMEWORK_COREAUDIO}           
    ${OF_FRAMEWORK_COREFOUNDATION}      
    ${OF_FRAMEWORK_COREMEDIA}           
    ${OF_FRAMEWORK_CORESERVICES}        
    ${OF_FRAMEWORK_COREVIDEO}           
    ${OF_FRAMEWORK_IOKIT}               
    ${OF_FRAMEWORK_OPENGL}              
    ${OF_FRAMEWORK_QUARTZCORE}          
    ${OF_FRAMEWORK_QTKIT}               
    ${OF_FRAMEWORK_SECURITY}       
)



#=== Configure Static OpenFrameworks Library
add_library(openFrameworksCore STATIC ${OF_CORE_SOURCE_FILES})

target_include_directories(openFrameworksCore PUBLIC
    ${OF_CORE_INCLUDE_PATHS}
    ${OF_LIB_INCLUDE_PATHS}
)

target_link_libraries(openFrameworksCore PUBLIC
    ${OF_CORE_LIBRARIES}
    ${OF_CORE_FRAMEWORKS}
    glfw #glfw is built externally to use a newer version than the one shipped precompiled by openframeworks
)

target_compile_definitions(openFrameworksCore PUBLIC
    OF_USING_STD_FS # we are using c++20 and the native std::filesystem instead of boost::filesystem
)

# some files need to be interpreted as objective c since they make use of macos framework headers
set_source_files_properties( ${OF_CORE_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "-x objective-c++" )



# fmod exists as a dynamic library and needs to be copied to the application package Frameworks folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OF_LIBS_DIRECTORY}/fmod/lib/osx/libfmod.dylib
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/libfmod.dylib
)

# needed to make a signing error go away in Xcode
set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep")



#package addons and core into one library
include(${CMAKE_CURRENT_LIST_DIR}/openframeworksaddons/addons.cmake)

add_library(openFrameworks INTERFACE)

target_link_libraries(openFrameworks INTERFACE
    openFrameworksCore
    openFrameworksAddons
)