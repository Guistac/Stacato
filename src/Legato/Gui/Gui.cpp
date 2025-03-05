#include <pch.h>
#include "Gui.h"

#include "config.h"

#define GLFW_INCLUDE_NONE //don't let GLFW include a gl loader, we're using GLAD
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
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
	bool b_goFullscreen;

	void makeMainWindowFullscreen(){
		b_goFullscreen = true;
	}

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

		//if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) Logger::error("Mouse Down {}", Timing::getProgramTime_seconds());
		
		//Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		
		//correct platform window and imgui viewport position desynchronization
		//when a window is moved and released, this checks if the internal position and actual window position match
		//if not it reads the actual window position and writes it to the internal position
		//if we dont use this, it can happen that the mouse cursor is offset from actual gui items
		//and this can cause a soft lock of the ui in that window
		auto& viewports = ImGui::GetCurrentContext()->Viewports;
		for(int i = 0; i < viewports.size(); i++){
			ImGuiViewportP* viewport = viewports[i];
			GLFWwindow* windowhandle = (GLFWwindow*)viewport->PlatformHandle;
			if(windowhandle == nullptr) continue;
			int platformPosX, platformPosY;
			glfwGetWindowPos(windowhandle, &platformPosX, &platformPosY);
			if(viewport->Pos.x != platformPosX || viewport->Pos.y != platformPosY){
				if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					//this forces resync of viewport position and window positions
					viewport->PlatformRequestMove = true;
					Logger::trace("Corrected viewport position to {} {}", platformPosX, platformPosY);
				}
			}
		}

		if(b_goFullscreen){
			b_goFullscreen = false;

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			int xpos, ypos, width, height;
			glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);

			GLFWmonitor* currentMonitor = glfwGetWindowMonitor(mainWindow);
			if(currentMonitor){
				glfwSetWindowMonitor(mainWindow, nullptr, 100, 100, width/3, height/3, GLFW_DONT_CARE);
				glfwMaximizeWindow(mainWindow);
				//glfwSetWindowAttrib(mainWindow, GLFW_DECORATED, GLFW_TRUE);
				//glfwSetWindowPos(mainWindow, 1, 1);
				Logger::info("Leaving Fullscreen");
			}
			else{
				glfwMaximizeWindow(mainWindow);
				glfwSetWindowMonitor(mainWindow, monitor, 0, 0, width, height, GLFW_DONT_CARE);
				Logger::info("Entering Fullscreen");
			}
		}

	}


	void initialize(){
			
		#if defined (STACATO_UNIX)
			//on ubuntu xorg desktop, touchscreen do not work with glfw decorated windows
			//too tired to understand why, an
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		#endif

		#if defined(STACATO_MACOS)
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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
		
		//OpenGL 4.1 is the latest supported version on MacOS
		if(ImGui_ImplOpenGL3_Init("#version 410 core")) Logger::debug("Graphics API: OpenGL 4.1");
		//OpenGL ES 3.1 is the latest version supported on Raspberry Pi 5
		else if(ImGui_ImplOpenGL3_Init("#version 300 es")) Logger::debug("Graphics API: OpenGL ES 3.0");
		else{
			Logger::error("ImGui failed to initialize graphics API");
			return;
		}

		std::string path = std::filesystem::current_path().string();
		Logger::debug("Current Path: {}", path);

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


