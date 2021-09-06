#include <pch.h>

#include "GuiWindow.h"

#include <glad/glad.h>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>

#include "Gui/Gui.h"

GLFWwindow* GuiWindow::window;
bool GuiWindow::b_shouldClose = false;

void GuiWindow::open(int w, int h) {
	glfwInit();
#ifdef MACOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif
	window = glfwCreateWindow(w, h, "Staccato", nullptr, nullptr);
	glfwSetWindowSizeCallback(window, onResize);
	glfwSetWindowCloseCallback(window, onClose);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	onInit();
}

void GuiWindow::refresh() {
	while (!b_shouldClose) {
		bool windowCloseRequested = glfwWindowShouldClose(window);
		if (windowCloseRequested) glfwSetWindowShouldClose(window, GLFW_FALSE);
		glfwMakeContextCurrent(window);
		glfwPollEvents();
        onRenderBegin();
		onRender(windowCloseRequested);
		onRenderEnd();
		glfwSwapBuffers(window);
	}
}

void GuiWindow::close() {
	onTerminate();
	glfwSetWindowShouldClose(window, GLFW_TRUE);
	glfwTerminate();
}

void GuiWindow::onInit() {
	IMGUI_CHECKVERSION();
	
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiNodeEditor::CreateContext();
	//ImNodes::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);

    float scaleTuning = 1.0;
    
#ifdef WIN32
    scaleTuning = 1.25;
#endif
    
#ifdef MACOS
    scaleTuning = 1.0;
#endif
    
    float scale = xScale * scaleTuning;

	Fonts::load(scale);

	ImGui::GetStyle().ScaleAllSizes(scale);
     
	ImGui::StyleColorsDark();
	ImGui::GetStyle().FrameRounding = 5.0;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410 core");
}

void GuiWindow::onTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGuiNodeEditor::DestroyContext();
	//ImNodes::DestroyContext();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void GuiWindow::onRenderBegin() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GuiWindow::onRenderEnd() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void GuiWindow::onRender(bool windowCloseRequested) {
	gui(windowCloseRequested);
}

void GuiWindow::onResize(GLFWwindow* window, int w, int h) {
	glfwMakeContextCurrent(window);
	glfwPollEvents();
	onRenderBegin();
	onRender(false);
	onRenderEnd();
	glfwSwapBuffers(window);
}

void GuiWindow::onClose(GLFWwindow* window) {

}

void GuiWindow::setShouldClose() {
	b_shouldClose = true;
}
