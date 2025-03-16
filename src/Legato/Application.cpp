#include <pch.h>

#include "Application.h"
#include "Application_Private.h"
#include "Logger.h"

#include "Gui/Gui_Private.h"

#include "config.h"
#include "Gui/Utilities/FileDialog.h"

#include <GLFW/glfw3.h>

#include <clocale>
#include <iostream>

namespace Application{
	
	std::function<bool(std::filesystem::path)> userInitializationFunction;
	std::function<bool()> userTerminationFunction;
	std::function<bool()> quitRequestFunction;
	void setInitializationFunction(std::function<bool(std::filesystem::path)> fn){ userInitializationFunction = fn; }
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

			std::cout << "launch arguments: " << argcount << std::endl;
			for(int i = 0; i < argcount; i++) std::cout << i << ":  " << args[i] << std::endl;
			
			std::filesystem::path launchPath = args[0];
			std::filesystem::path absoluteExecutablePath;
			if(launchPath.is_relative()) {
				std::string relativeLaunchPath = launchPath.string();
				if(relativeLaunchPath.substr(0, 2) == "./") relativeLaunchPath.erase(0, 2);
				absoluteExecutablePath = std::filesystem::current_path() / relativeLaunchPath;
			}
			else{
				absoluteExecutablePath = launchPath;
			}

			std::cout << "Executable Path: 	" << absoluteExecutablePath.string() << std::endl;

			std::filesystem::path executableDirectory = absoluteExecutablePath.parent_path();
			std::filesystem::path debugResourceFolder = executableDirectory.parent_path() / "dir/Resources";
			std::filesystem::path releaseResourcesFolder = executableDirectory / "Resources";

			if(std::filesystem::exists(debugResourceFolder)){
				std::filesystem::current_path(debugResourceFolder);
			}else if(std::filesystem::exists(releaseResourcesFolder)){
				std::filesystem::current_path(releaseResourcesFolder);
			}else{
				std::cout << "[ERROR] Resources folder is missing." << std::endl;
				std::cout << "[ERROR] Application cannot start." << std::endl;
				std::cin.get();
				return;
			}
			std::cout << "Working directory: " << std::filesystem::current_path().string() << std::endl;

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
		
		//====== APP IS FULLY INITIALIZED
		
		//——— il the app launched with a file path, open that file in the workspace after the app is initialized
		std::filesystem::path applicationLaunchFilePath;
		#if defined(STACATO_MACOS)
			if(const char* path = glfwGetOpenedFilePath()) applicationLaunchFilePath = path;
		#elif defined(STACATO_UNIX)
			if(argcount >= 2) applicationLaunchFilePath = args[1];
		#endif

		//open the file path in the workspace
		if(!applicationLaunchFilePath.empty()){
			Logger::info("[Application] Application was launched with file path {}", applicationLaunchFilePath.string());
		}else{
			if(Workspace::getLastLoadedFilePath(applicationLaunchFilePath)){
				Logger::info("Found last loaded file: {}", applicationLaunchFilePath.string());
			}else Logger::info("Could not find last opened file");
		}

		//——— User Initialization
		userInitializationFunction(applicationLaunchFilePath);		
		
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
	
	void requestShutdown(){
		std::string command = "sudo poweroff";
		FILE* pipe = popen(command.c_str(), "r");
		if(!pipe) Logger::critical("Failed to execute bash script");
		/*
		char buffer[64];
		fgets(buffer, sizeof(buffer), pipe);
		try{
			std::stoi(buffer);
		}catch(std::invalid_argument& error){
			
		}
		pclose(pipe);
		*/
	}

};
	
