#include <pch.h>

#include "ApplicationWindow.h"
#include "config.h"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define OPENGL_VERSION_STRING "#version 410 core"

#define GLFW_INCLUDE_NONE //necessary or glfw will include its own GL loader
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "Gui.h"
#include "Gui/Utilities/FileDialog.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Images.h"
#include "Project/Project.h"

#include "Visualizer/Visualizer.h"

namespace ApplicationWindow {

	GLFWwindow* window;
	char windowName[256] = "Stacato";
	bool b_windowNameShowUnsavedModifications = false;

	bool b_shouldClose = false;

	double macOsTrackpadZoomDelta = 0.0;
	double macOsTrackpadRotateDelta = 0.0;

	bool b_launchedByOpenedFile = false;
	char openedFilePath[512];


	void init() {
		
		pthread_setname_np("Gui Thread");
		
		Timing::start();
		Random::initialize();
		
	#ifdef STACATO_MACOS
	#ifdef STACATO_DEBUG
		//for debug builds, don't change the working directory
		//resources and debug project are loaded and saved to the repository's dir/ folder
		glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, false);
	#else
		//for release builds, change the working directory to the resources folder inside the .app bundle
		glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, true);
	#endif
	#endif
		
		glfwInit(); //this also sets the working directory on macos builds
		FileDialog::init();
		
	#ifdef STACATO_MACOS
		//if the app was launched by opening a file path, remember the file details for later user
		const char* path = glfwGetOpenedFilePath();
		b_launchedByOpenedFile = path != nullptr;
		if(b_launchedByOpenedFile) strcpy(openedFilePath, path);
		
		//we delete this file on each startup since we don't use it and it sometimes causes loading problems with imguinodeeditor
		std::filesystem::path nodeEditorJsonFile = "NodeEditor.json";
		std::filesystem::remove(nodeEditorJsonFile);
		
	#endif
		
	#ifdef STACATO_WIN32
	#ifdef STACATO_RELEASE
		//for windows release builds, set working directory to "Resources" folder located next to executable
		std::string defaultWorkingDirectory = std::filesystem::current_path().string();
		std::filesystem::current_path(defaultWorkingDirectory + "/Resources");
	#endif
	#endif
		
		Logger::init(); //Logger is initialized after working directory is defined to have log file access
		Logger::critical("Stacato Version {}.{} {} ({})", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME, STACATO_BUILD_TYPE);
	}






	void terminate() {
	   Logger::terminate();
	   FileDialog::terminate();
	   glfwTerminate();
	}






	void open() {
		
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
		
	#ifdef STACATO_MACOS
		glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
	#endif
							
		glm::ivec2 workPos, workSize;
		glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &workPos.x, &workPos.y, &workSize.x, &workSize.y);
		
		//this opens the main application window and creates the main opengl context
		window = glfwCreateWindow(workSize.x, workSize.y, windowName, nullptr, nullptr);
		glfwSetWindowPos(window, workPos.x, workPos.y);

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
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		
        
        float displayScale;
        glfwGetWindowContentScale(window, &displayScale, nullptr);
        float guiScale;
        
        #ifdef STACATO_WIN32
        guiScale = 1.25 * displayScale; //best scaling to match Windows Gui
        #endif
        #ifdef STACATO_MACOS
        if(displayScale == 1.0) guiScale = 1.4;      //best scaling to match non retina MacOS Gui
        else if(displayScale == 2.0) guiScale = 0.7; //best scaling to match retina MacOS Gui
        else guiScale = 1.0;
        #endif
        
		Logger::debug("Display Scale: {} Gui Scale: {}", displayScale, guiScale);
		Fonts::load(guiScale);
		Images::load();
		
		ImGui::GetStyle().ScaleAllSizes(guiScale);
		ImGui::StyleColorsDark();
		
		//initialize glfw & opengl backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(OPENGL_VERSION_STRING);
		Environnement::StageVisualizer::initialize(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
				
		//============ UPDATE LOOP ============
		while (!b_shouldClose) update();
		
		//Gui Shutdown
		Environnement::StageVisualizer::terminate();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
	}





	void update(){
				
		//if a window close request was issued don't close the window but notify the gui and let the user confirm the request
		if (glfwWindowShouldClose(window)) {
			glfwSetWindowShouldClose(window, GLFW_FALSE);
			requestQuit();
		}
		
		//with multiple windows/viewports the context of the main window needs to be set on each frame
		glfwMakeContextCurrent(window);
		
		//update window user inputs
		double d_previousZoom = macOsTrackpadZoomDelta;
		double d_previousRotation = macOsTrackpadRotateDelta;
		glfwPollEvents();
		if(d_previousZoom == macOsTrackpadZoomDelta) macOsTrackpadZoomDelta = 0.0;
		if(d_previousRotation == macOsTrackpadRotateDelta) macOsTrackpadRotateDelta = 0.0;
		
		//begin new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		//execute out gui
		Gui::draw();

		//render and display the gui
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
		else Gui::QuitApplicationPopup::get()->open();
	}

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
			if(window){
				static char windowNameUnsaved[333];
				sprintf(windowNameUnsaved, "%s (Unsaved Changes)", windowName);
				glfwSetWindowTitle(window, windowNameUnsaved);
			}
			b_windowNameShowUnsavedModifications = true;
		}
	}

	void hideUnsavedModifications(){
		if(b_windowNameShowUnsavedModifications){
			if(window) glfwSetWindowTitle(window, windowName);
			b_windowNameShowUnsavedModifications = false;
		}
	}

}
