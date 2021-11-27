#include <pch.h>

#include "config.h"

#include "ApplicationWindow.h"

#ifdef STACATO_USE_MAGNUM_ENGINE
	//we include magnums gl context first since it loads the opengl functions
	#include <Magnum/Platform/GLContext.h>
	//we include glfw after so it doesn't do any gl loading
	#include <glfw/glfw3.h>
	//we declare the following so imgui doesn't try to do any importing of external gl loaders (config.h is just a dummy import)
	#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM "config.h"
	#include <backends/imgui_impl_opengl3.cpp>
	#include <backends/imgui_impl_glfw.cpp>
#else
	//Use the GLAD Gl Loader and disable Magnum
	#include <glad/glad.h>
	#define IMGUI_IMPL_OPENGL_LOADER_GLAD
	#include <backends/imgui_impl_opengl3.cpp>
	#include <backends/imgui_impl_glfw.cpp>
	#include <GLFW/glfw3.h>
#endif

#include <implot.h>
#include "Gui/NodeEditor/NodeEditorGui.h"
#include "Gui/Utilities/FileDialog.h"
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"


namespace ApplicationWindow {

	GLFWwindow* window;
	bool b_closeRequested = false;
	bool b_shouldClose = false;

#ifdef STACATO_WIN32
    float scaleTuning = 1.25;
#elseif STACATO_MACOS
    float scaleTuning = 0.75;
#else
    float scaleTuning = 0.5;
#endif

	void init() {
#ifdef STACATO_MACOS
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
        //glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);
#endif
		glfwInit();
		FileDialog::init();
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
		//this opens the main application window and creates the main opengl context
		window = glfwCreateWindow(w, h, "Stacato", nullptr, nullptr);

		//set a lamdba callback to have the window draw while being resized
		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int w, int h) {
				glfwPollEvents();
				render();
				glfwSwapBuffers(window);
		});

		//activate the opengl context
		glfwMakeContextCurrent(window);

		//active V-Sync
		glfwSwapInterval(1);

#ifndef STACATO_USE_MAGNUM_ENGINE
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

		Logger::info("OpenGL Version {}", (const char*)glGetString(GL_VERSION));

		//initialize gl context for magnum engine
#ifdef STACATO_USE_MAGNUM_ENGINE
		Magnum::Platform::GLContext magnumGlContext;
		//magnumGlContext.
#endif

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiNodeEditor::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		float xScale, yScale;
		glfwGetWindowContentScale(window, &xScale, &yScale);
		float scale = xScale * scaleTuning;
        Logger::info("Display Scale: {}  Scale Tuning: x{}  Gui Scale: {}", xScale, scaleTuning, scale);
		Fonts::load(scale);
		ImGui::GetStyle().ScaleAllSizes(scale);
		ImGui::StyleColorsDark();
		ImGui::GetStyle().FrameRounding = 5.0;

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410 core");
		//ImGui_ImplOpenGL3_Init("#version 460 core");

		//=====================================
		//============ RENDER LOOP ============
		//=====================================
		while (!b_shouldClose) {
			if (glfwWindowShouldClose(window)) {
				glfwSetWindowShouldClose(window, GLFW_FALSE);
				requestClose();
			}
			glfwMakeContextCurrent(window); //with multiple windows (docking) the context needs to be set each time
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

}
