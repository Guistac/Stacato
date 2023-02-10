#include <pch.h>
#include "Gui.h"

namespace NewGui{

std::function<void()> userInitializationFunction;
std::function<void()> userPreFrameFunction;
std::function<void()> userPostFrameFunction;
std::function<void()> userTerminationFunction;

void setInitializationFunction(std::function<void()> fn){ userInitializationFunction = fn; }
void setPreFrameFunction(std::function<void()> fn){ userPreFrameFunction = fn; }
void setPostFrameFunction(std::function<void()> fn){ userPostFrameFunction = fn; }
void setTerminationFunction(std::function<void()> fn){ userTerminationFunction = fn; }

void initialize(){
	//initialize stuff
	//-GLFW
	//-ImGui
	//-ImGuiLibraries
	
	//user initialization : load fonts, images, etc
	userInitializationFunction();
}

void drawFrame(){
	//do backend stuff
	//new glfw frame
	//poll input events
	//new imgui frame
	
	userPreFrameFunction();
	
	//do windowing stuff
	//draw all windows
	//draw all popups
	
	userPostFrameFunction();
	
	//do backend stuff
	//end imgui frame
	//end glfw frame
}

void terminate(){
	//terminate user stuff : unload stuff
	userTerminationFunction();
	
	//do backend termination
	//terminate imgui
	//terminate GLFW
}

};
