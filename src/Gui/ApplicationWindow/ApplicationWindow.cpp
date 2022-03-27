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
#include "Gui.h"
#include "Gui/Utilities/FileDialog.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Project/Project.h"

#include <implot.h>

#include "UnitT.h"

namespace ApplicationWindow {

GLFWwindow* window;
char windowName[256] = "Stacato";
bool b_windowNameShowUnsavedModifications = false;

bool b_closeRequested = false;
bool b_shouldClose = false;

double macOsTrackpadZoomDelta = 0.0;
double macOsTrackpadRotateDelta = 0.0;

bool b_launchedByOpenedFile = false;
char openedFilePath[512];

#ifdef STACATO_WIN32
float scaleTuning = 1.25; //best scaling to match Windows Gui
#endif
#ifdef STACATO_MACOS
float scaleTuning = 0.7; //best scaling to match MacOS Gui
#endif


void init() {
	
	Timing::start();
	Random::initialize();
	glfwInit();
	FileDialog::init();
	
#ifdef STACATO_MACOS
	const char* path = glfwGetOpenedFilePath();
	b_launchedByOpenedFile = path != nullptr;
	if(b_launchedByOpenedFile) strcpy(openedFilePath, path);
#endif
	
#ifdef STACATO_WIN32
	//for windows builds, set working directory to "Resources" folder located next to executable
	std::string defaultWorkingDirectory = std::filesystem::current_path().string();
	std::filesystem::current_path(defaultWorkingDirectory + "/Resources");
#endif
	
	//Logger is initialized after working directory is defined to have log file access
	Logger::init();
	Logger::critical("Stacato Version {}.{} {} ({})", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME, STACATO_BUILD_TYPE);
	
}




void terminate() {
	Logger::terminate();
	FileDialog::terminate();
	glfwTerminate();
}




void open(int w, int h) {
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
	
#ifdef STACATO_MACOS
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif
	
	//this opens the main application window and creates the main opengl context
	window = glfwCreateWindow(w, h, windowName, nullptr, nullptr);

	//set window callbacks that are not handled by imgui glfw backend
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int w, int h) { update(); });
	glfwSetOpenFileCallback([](const char* filePath){ onFileOpen(filePath); });
	glfwSetZoomCallback(window, [](GLFWwindow* window, double zoom){ macOsTrackpadZoomDelta = zoom; });
	glfwSetRotateCallback(window, [](GLFWwindow* window, double rotation){ macOsTrackpadRotateDelta = rotation; });
	
	//activate the opengl context & activate vsync
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	
	//Load OpenGL Functions
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) return false;
	Logger::debug("OpenGL Version {}", (const char*)glGetString(GL_VERSION));
	
	//initialize gui contexts
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	
	//configure imgui
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
	
	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);
	float scale = xScale * scaleTuning;
	Logger::debug("Display Scale: {}  Scale Tuning: x{}  Gui Scale: {}", xScale, scaleTuning, scale);
	Fonts::load(scale);
	ImGui::GetStyle().ScaleAllSizes(scale);
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 5.0;
	
	//initialize glfw & opengl backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(OPENGL_VERSION_STRING);

	//============ UPDATE LOOP ============
	while (!b_shouldClose) update();

	//Gui Shutdown
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}


void update(){
	
	if (glfwWindowShouldClose(window)) {
		//if a window close request was issued
		//don't close the window but notify the gui and let the user decide on closing the window
		glfwSetWindowShouldClose(window, GLFW_FALSE);
		requestQuit();
	}
	
	double d_previousZoom = macOsTrackpadZoomDelta;
	double d_previousRotation = macOsTrackpadRotateDelta;
	
	//with multiple windows/viewports the context of the main window needs to be set on each frame
	glfwMakeContextCurrent(window);
	
	glfwPollEvents();
	
	if(d_previousZoom == macOsTrackpadZoomDelta) macOsTrackpadZoomDelta = 0.0;
	if(d_previousRotation == macOsTrackpadRotateDelta) macOsTrackpadRotateDelta = 0.0;
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	Gui::draw();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
	
	//Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}


//request the app to quit, will open popup
void requestQuit() {
	if(Project::canCloseImmediately()) quit();
	else b_closeRequested = true;
}

//cancel the quit request
void cancelQuitRequest() { b_closeRequested = false; }

//did we try to quit the app
bool isQuitRequested() { return b_closeRequested; }

//definitely close the app
void quit() { b_shouldClose = true; }



//was the app launched by opening a file
bool wasLaunchedByOpeningFile(){ return b_launchedByOpenedFile; }

//get the opened file path
const char* getOpenedFilePath(){ return openedFilePath; }

void onFileOpen(const char* filePath){ Project::load(filePath); }

//get trackpad gesture delta values
double getMacOsTrackpadZoom(){ return macOsTrackpadZoomDelta; }
double getMacOsTrackpadRotate(){ return macOsTrackpadRotateDelta; }



void setWindowName(const char* name){
	sprintf(windowName, "%s", name);
	if(window) glfwSetWindowTitle(window, name);
}

void showUnsavedModifications(){
	if(!b_windowNameShowUnsavedModifications){
		static char windowNameUnsaved[333];
		sprintf(windowNameUnsaved, "%s (Unsaved Changes)", windowName);
		glfwSetWindowTitle(window, windowNameUnsaved);
		b_windowNameShowUnsavedModifications = true;
	}
}

void hideUnsavedModifications(){
	if(b_windowNameShowUnsavedModifications){
		glfwSetWindowTitle(window, windowName);
		b_windowNameShowUnsavedModifications = false;
	}
}




}
