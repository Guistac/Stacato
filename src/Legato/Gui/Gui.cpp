#include <pch.h>
#include "Gui.h"

#include "config.h"

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define OPENGL_VERSION_STRING "#version 410 core"

#define GLFW_INCLUDE_NONE //don't let GLFW include a gl loader, we're using GLAD
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot.h>

#include "Legato/Application.h"
#include "Legato/Application_Private.h"
#include "Gui_Private.h"
#include "Window.h"

namespace Legato::Gui{

	std::function<void()> userInitializationFunction;
	std::function<void()> userGuiSubmitFunction;
	std::function<void()> userTerminationFunction;
	void setInitializationFunction(std::function<void()> fn){ userInitializationFunction = fn; }
	void setGuiSubmitFunction(std::function<void()> fn){ userGuiSubmitFunction = fn; }
	void setTerminationFunction(std::function<void()> fn){ userTerminationFunction = fn; }

	GLFWwindow* mainWindow;

	float guiScale;
	float getScale(){ return guiScale; }


	void drawFrame(){
		
		//============ UPDATE LOOP ============
		//if a window close request was issued don't close the window but notify the gui and let the user confirm the request
		if (glfwWindowShouldClose(mainWindow)) {
			glfwSetWindowShouldClose(mainWindow, GLFW_FALSE);
			Application::requestQuit();
		}
		
		//with multiple windows/viewports the context of the main window needs to be set on each frame
		glfwMakeContextCurrent(mainWindow);
		
		//begin new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		//execute user gui
		userGuiSubmitFunction();
		
		//draw windows, popups and apply layouts
		WindowManager::update();
		
		//render and display the gui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(mainWindow);
		
		//Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		
		/*
		//NOTE: this causes jittering and inconsistencies when dragging viewport windows on linux...
		//		initially this was a fix to avoid problems on macos when dragging windows above the menu bar on second monitor
		//		we'll disable it for now, i need to check if this issue was resolved on macos
		//		(needs to include imgui_internal.h)

		//correct platform window and imgui viewport position desynchronization
		auto& viewports = ImGui::GetCurrentContext()->Viewports;
		for(int i = 0; i < viewports.size(); i++){
			ImGuiViewportP* viewport = viewports[i];
			GLFWwindow* windowhandle = (GLFWwindow*)viewport->PlatformHandle;
			if(windowhandle == nullptr) continue;
			int platformPosX, platformPosY;
			glfwGetWindowPos(windowhandle, &platformPosX, &platformPosY);
			if(viewport->Pos.x != platformPosX || viewport->Pos.y != platformPosY){
				//this forces resync of viewport position and window positions
				viewport->PlatformRequestMove = true;
			}
		}
		*/

	}


	void initialize(){
			
		//this is needed on MacOs, not on unix...
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
		
		#if defined(STACATO_MACOS)
			glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
		#endif
							
		//this opens the main application window and creates the main opengl context
		//open window on main monitor, max window size
		glm::ivec2 workPos, workSize;
		glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &workPos.x, &workPos.y, &workSize.x, &workSize.y);
		mainWindow = glfwCreateWindow(workSize.x, workSize.y, "Stacato", nullptr, nullptr);
		glfwSetWindowPos(mainWindow, workPos.x, workPos.y);

		//set window callbacks that are not handled by imgui glfw backend
		glfwSetWindowSizeCallback(mainWindow, [](GLFWwindow* window, int w, int h) { drawFrame(); });
		glfwSetZoomCallback(mainWindow, [](GLFWwindow* window, double zoom){ Application::updateTrackpadZoom(zoom); });
		glfwSetRotateCallback(mainWindow, [](GLFWwindow* window, double rotation){ Application::updateTrackpadRotation(rotation); });
		
		//activate the opengl context & activate vsync
		glfwMakeContextCurrent(mainWindow);
		glfwSwapInterval(1);
		
		if(!gladLoadGL()){
			Logger::error("Failed to load GLAD bindings");
			return;
		}
		Logger::debug("OpenGL Version {}", (const char*)glGetString(GL_VERSION));
		
		//initialize gui contexts
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		
		//configure imgui
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		
		//figure out gui scale
		#ifdef STACATO_WIN32
			float displayScale;
			glfwGetWindowContentScale(window, &displayScale, nullptr);
			guiScale = 1.25f * displayScale; //best scaling to match Windows Gui
		#endif
		#ifdef STACATO_MACOS
			guiScale = 1.4f; //best scaling to match MacOS Gui
		#endif
		#ifdef STACATO_UNIX
			guiScale = 1.4f;
		#endif


		ImGui::GetStyle().ScaleAllSizes(guiScale);
		Logger::debug("Gui Scale: {}", guiScale);
		
		//initialize glfw & opengl backends
		ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
		ImGui_ImplOpenGL3_Init(OPENGL_VERSION_STRING);

		std::string path = std::filesystem::current_path().string();
		Logger::warn("Current Path: {}", path);

		//user gui initialization
		userInitializationFunction();
		
	}


	void terminate(){
		//terminate user stuff : unload stuff
		userTerminationFunction();
		
		//Gui Shutdown
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
	}

};


