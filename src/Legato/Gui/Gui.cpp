#include <pch.h>
#include "Gui.h"

#include "Legato/Application.h"
#include "Legato/Application_Private.h"
#include "Gui_Private.h"

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


#include <imgui_internal.h>



namespace Legato::Gui{

std::function<void()> userInitializationFunction;
std::function<void()> userGuiSubmitFunction;
std::function<void()> userTerminationFunction;
void setInitializationFunction(std::function<void()> fn){ userInitializationFunction = fn; }
void setGuiSubmitFunction(std::function<void()> fn){ userGuiSubmitFunction = fn; }
void setTerminationFunction(std::function<void()> fn){ userTerminationFunction = fn; }

GLFWwindow* window;
std::string windowName = "Stacato";

float scale;
float getScale(){ return scale; }

void drawFrame();


void initialize(){
		
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
	#if defined(STACATO_MACOS)
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
	#endif
						
	//this opens the main application window and creates the main opengl context
	//open window on main monitor, max window size
	glm::ivec2 workPos, workSize;
	glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &workPos.x, &workPos.y, &workSize.x, &workSize.y);
	window = glfwCreateWindow(workSize.x, workSize.y, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowPos(window, workPos.x, workPos.y);

	//set window callbacks that are not handled by imgui glfw backend
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int w, int h) { drawFrame(); });
	glfwSetZoomCallback(window, [](GLFWwindow* window, double zoom){ Application::updateTrackpadZoom(zoom); });
	glfwSetRotateCallback(window, [](GLFWwindow* window, double rotation){ Application::updateTrackpadRotation(rotation); });
	
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
	
	//figure out gui scale
	#ifdef STACATO_WIN32
	float displayScale;
	glfwGetWindowContentScale(window, &displayScale, nullptr);
	scale = 1.25f * displayScale; //best scaling to match Windows Gui
	#endif
	#ifdef STACATO_MACOS
	scale = 1.4f; //best scaling to match MacOS Gui
	#endif
	ImGui::GetStyle().ScaleAllSizes(scale);
	Logger::debug("Gui Scale: {}", scale);
	
	//initialize glfw & opengl backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(OPENGL_VERSION_STRING);

	//user gui initialization
	userInitializationFunction();
	
}


void drawFrame(){
	
	//============ UPDATE LOOP ============
	//if a window close request was issued don't close the window but notify the gui and let the user confirm the request
	if (glfwWindowShouldClose(window)) {
		glfwSetWindowShouldClose(window, GLFW_FALSE);
		Application::requestQuit();
	}
	
	//with multiple windows/viewports the context of the main window needs to be set on each frame
	glfwMakeContextCurrent(window);
	
	//begin new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	//execute user gui
	userGuiSubmitFunction();
	
	
		
	drawWindows();
	drawPopups();
	
	//we should do WindowManager update here
	
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
