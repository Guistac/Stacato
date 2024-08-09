#include <pch.h>

#include "Application.h"
#include "Application_Private.h"
#include "Logger.h"

#include "Gui/Gui_Private.h"

#include "config.h"
#include "Gui/Utilities/FileDialog.h"

#include <GLFW/glfw3.h>

#include <clocale>

namespace Application{
	
	std::function<bool()> userInitializationFunction;
	std::function<bool()> userTerminationFunction;
	std::function<bool()> quitRequestFunction;
	void setInitializationFunction(std::function<bool()> fn){ userInitializationFunction = fn; }
	void setTerminationFunction(std::function<bool()> fn){ userTerminationFunction = fn; }
	void setQuitRequestFunction(std::function<bool()> fn){ quitRequestFunction = fn; }

	bool b_running = false;
	
	float trackpadZoomDelta = 0.0;
	float trackpadRotationDelta = 0.0;
	void updateTrackpadZoom(float zoom){ trackpadZoomDelta = zoom; }
	void updateTrackpadRotation(float rotation){ trackpadRotationDelta = rotation; }
	float getTrackpadZoomDelta(){ return trackpadZoomDelta; }
	float getTrackpadRotationDelta(){ return trackpadRotationDelta; }
	
	void updateInputs(){
		//update window user inputs
		//double d_previousZoom = macOsTrackpadZoomDelta;
		//double d_previousRotation = macOsTrackpadRotateDelta;
		glfwPollEvents();
		//if(d_previousZoom == macOsTrackpadZoomDelta) macOsTrackpadZoomDelta = 0.0;
		//if(d_previousRotation == macOsTrackpadRotateDelta) macOsTrackpadRotateDelta = 0.0;
	}
	
	void run(int argcount, const char ** args){
		
		//——— this thread will do general setup and then loop while drawing the gui
		//pthread_setname_np("Gui Thread");
		
		//====== START INITIALIZING APP
		
		//Set Working directory for Debug and Release builds
		//some of these settings are IDE specific and set through cmake
		//other settings are library and platform specific and are set here

		#if defined(STACATO_MACOS)
			#if defined(STACATO_DEBUG)
				//for debug builds, don't change the working directory
				//resources and debug project are loaded and saved to the repository's dir/ folder
				//this setting is done with the cmake script
				glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, false);
			#else
				//for release builds, let glfw change the working directory to the resources folder inside the .app bundle
				glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, true);
			#endif

		#elif defined(STACATO_WIN32) && defined(STACATO_RELEASE)
			//for windows release builds, set working directory to "Resources" folder located next to executable
			std::string defaultWorkingDirectory = std::filesystem::current_path().string();
			std::filesystem::current_path(defaultWorkingDirectory + "/Resources");
			//the working directory for debug builds is set through cmake

		#elif defined(STACATO_UNIX)

			//arg[0] can be an absolute or relative path
			//if arg[0] contains the current path, then it is an absolute path
			//else arg[0] is a relative path and we append it to the current path to get the absolute executable path
			std::string executableLaunchPath = args[0];
			size_t currentPathInLaunchPath = executableLaunchPath.find(std::filesystem::current_path().string());
			std::filesystem::path absolutePathToExecutable;
			if(currentPathInLaunchPath != std::string::npos) absolutePathToExecutable = args[0];
			else absolutePathToExecutable = std::filesystem::current_path().string() + "/" + executableLaunchPath;
			std::string workingDirectory;
			#if defined(STACATO_DEBUG)
				//for debug builds, set the working directory to the dir/ folder in the source tree
				workingDirectory = absolutePathToExecutable.parent_path().parent_path().string() + "/dir/Resources";
			#else
				//for release builds, set the working directory to the Resources folder in the same directory as the executable
				workingDirectory = absolutePathToExecutable.parent_path().string() + "/Resources";
			#endif
			std::filesystem::current_path(workingDirectory);

		#endif

		//——— initialize glfw for windowing and user inputs
		glfwInit(); //this also sets the working directory to .app/Resources on MacOs builds
		
		glfwSetOpenFileCallback([](const char* filePath){
			if(filePath) Workspace::openFile(std::filesystem::path(filePath));
		});
		

		//——— Logger is initialized after working directory is defined to have log file access
		NewLogger::initialize();
		Logger::info("Stacato Version {}.{} {} ({})", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME, STACATO_BUILD_TYPE);
		
		//——— core library initialization
		Random::initialize();
		FileDialog::init(); //this sets locale on unix for some reason...

		//——— set locale, this sets decimal points to . and not ,
		std::setlocale(LC_NUMERIC, "C");
		
		//——— Gui Initialization
		Legato::Gui::initialize();
		
		//——— User Initialization
		userInitializationFunction();		
		
		//====== APP IS FULLY INITIALIZED
		
		//——— il the app launched with a file path, open that file in the workspace after the app is initialized
		std::filesystem::path applicationLaunchFilePath;
		#if defined(STACATO_MACOS)
			if(const char* path = glfwGetOpenedFilePath()) applicationLaunchFilePath = path;
		#endif
		//open the file path in the workspace
		if(!applicationLaunchFilePath.empty()){
			Logger::info("[Application] Application was launched with file path {}", applicationLaunchFilePath.string());
			Workspace::openFile(applicationLaunchFilePath);
		}
		
		//====== RUN APPLICATION
		
		//——— Main Gui Loop
		b_running = true;
		while(b_running) {
			updateInputs();
			Legato::Gui::drawFrame();
		}
		
		
		//====== TERMINATE APPLICATION
		
		//——— User termination
		userTerminationFunction();
		
		//——— Gui termination
		Legato::Gui::terminate();
		
		//——— Core library termination
		FileDialog::terminate();
		glfwTerminate();
		
		//——— Logger terminates last to be able to log until total shutdown
		Logger::terminate();
		
	}
	
	
	void requestQuit(){
		if(quitRequestFunction()) quitImmediately();
	}
	
	
	void quitImmediately(){ b_running = false; }
	
};
	
