#include <pch.h>

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define OPENGL_VERSION_STRING "#version 410 core"

#define GLFW_INCLUDE_NONE //necessary or glfw will include its own GL loader
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "config.h"
#include "ApplicationWindow.h"
#include "Gui/NodeEditor/NodeEditorGui.h"
#include "Gui/Utilities/FileDialog.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include <implot.h>

#include <iostream>

namespace ApplicationWindow {

GLFWwindow* window;
bool b_closeRequested = false;
bool b_shouldClose = false;

bool b_launchedByOpenedFile = false;
char openedFilePath[512];

float scaleTuning = 1.0;

char windowName[256] = "Stacato";

GLFWwindow* getGlfwWindow(){
	return window;
}

void init() {
	
#ifdef STACATO_WIN32
//#ifndef STACATO_DEBUG
	//for release build, set working directory to "Resources" folder located next to executable
	//the working directory for debug builds is located in the repositories dir/ folder
	std::string defaultWorkingDirectory = std::filesystem::current_path().string();
	std::filesystem::current_path(defaultWorkingDirectory + "/Resources");
//#endif
#endif
	
	/*
#ifdef STACATO_MACOS
#ifdef STACATO_DEBUG
	//prevent glfw from setting the working directory inside .app package resources folder
	//the working directory for debug builds is located in the repositories dir/ folder
	glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
#endif
	 */
	
	glfwInit();
	FileDialog::init();
	
	const char* path = glfwGetOpenedFilePath();
	b_launchedByOpenedFile = path != nullptr;
	if(b_launchedByOpenedFile) strcpy(openedFilePath, path);
	
}




void terminate() {
	FileDialog::terminate();
	glfwTerminate();
}




void open(int w, int h) {
	
#ifdef STACATO_MACOS
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif
#ifdef STACATO_WIN32
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
	
	//this opens the main application window and creates the main opengl context
	window = glfwCreateWindow(w, h, windowName, nullptr, nullptr);

	//set a lamdba callback to have the window draw while being resized
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int w, int h) {
			glfwPollEvents();
			render();
			glfwSwapBuffers(window);
	});
	
	glfwSetOpenFileCallback([](const char* filename){
		Logger::critical("Opening File: {}", filename);
	});
	
	//activate the opengl context
	glfwMakeContextCurrent(window);

	//activate V-Sync
	glfwSwapInterval(1);
	
	glewExperimental = GL_TRUE; //absolutely necessary to load the correct opengl functions for some reason
	if(glewInit() != GLEW_OK) return false;
	
	Logger::debug("OpenGL Version {}", (const char*)glGetString(GL_VERSION));
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiNodeEditor::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
	
#ifdef STACATO_WIN32
	//best scaling to match Windows Gui
	scaleTuning = 1.25;
#endif
#ifdef STACATO_MACOS
	//best scaling to match MacOS Gui
	scaleTuning = 0.7;
#endif
	
	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);
	
	float scale = xScale * scaleTuning;
	Logger::debug("Display Scale: {}  Scale Tuning: x{}  Gui Scale: {}", xScale, scaleTuning, scale);
	Fonts::load(scale);
	
	ImGui::GetStyle().ScaleAllSizes(scale);
	
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0;
	
	ImGui_ImplGlfw_InitForOpenGL(window, true);

#ifdef STACATO_MACOS
	ImGui_ImplOpenGL3_Init("#version 410 core");
#endif
#ifdef STACATO_WIN32
	ImGui_ImplOpenGL3_Init("#version 460 core");
#endif

	//=====================================
	//============ RENDER LOOP ============
	//=====================================
	while (!b_shouldClose) {
		if (glfwWindowShouldClose(window)) {
			//if a window close request was issued
			//don't close the window but notify the gui and let the user decide on closing the window
			glfwSetWindowShouldClose(window, GLFW_FALSE);
			requestClose();
		}
		//with multiple windows/viewports the context of the main window needs to be set on each frame
		glfwMakeContextCurrent(window);
		glfwPollEvents();
		render();
		glfwSwapBuffers(window);
	}
	//=====================================
	//============ RENDER LOOP ============
	//=====================================

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGuiNodeEditor::DestroyContext();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}



void requestClose() {
	b_closeRequested = true;
}




void cancelCloseRequest() {
	b_closeRequested = false;
}




bool isCloseRequested() {
	return b_closeRequested;
}




void close() {
	b_shouldClose = true;
}




void render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	onRender();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	//Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}




void onRender() {
	drawGui();
}




float getScaleTuning(){
	return scaleTuning;
}

void setWindowName(const char* name){
	if(window) glfwSetWindowTitle(window, name);
	else sprintf(windowName, "%s", name);
}



bool wasLaunchedByOpeningFile(){
	return b_launchedByOpenedFile;
}

const char* getOpenedFilePath(){
	return openedFilePath;
}


}
