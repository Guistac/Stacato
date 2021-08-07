#include "GuiWindow.h"

#ifdef WIN32
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>

#ifdef WIN32
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.cpp>
#endif

#include <backends/imgui_impl_glfw.cpp>
#include <imgui.h>
#include <implot.h>

#include "Gui.h"

GLFWwindow* GuiWindow::window;

ImFont* GuiWindow::robotoBold15;
ImFont* GuiWindow::robotoLight15;
ImFont* GuiWindow::robotoRegular15;

ImFont* GuiWindow::robotoBold20;
ImFont* GuiWindow::robotoLight20;
ImFont* GuiWindow::robotoRegular20;

ImFont* GuiWindow::robotoBold42;
ImFont* GuiWindow::robotoLight42;
ImFont* GuiWindow::robotoRegular42;

void GuiWindow::open(int w, int h) {
	glfwInit();
	window = glfwCreateWindow(w, h, "FieldbusDev", nullptr, nullptr);
	glfwSetWindowSizeCallback(window, onResize);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
    
#ifdef WIN32
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif
    
	onInit();
}

void GuiWindow::refresh() {
	while (!glfwWindowShouldClose(window)) {
		glfwMakeContextCurrent(window);
		glfwPollEvents();
		onRenderBegin();
		onRender(false);
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

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);

	float scaleTuning = 1.25;

	float scale = xScale * scaleTuning;
	robotoRegular15 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Regular.ttf", 15.0f * scale);
	robotoBold15 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Bold.ttf", 15.0f * scale);
	robotoLight15 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Light.ttf", 15.0f * scale);
	robotoRegular20 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Regular.ttf", 20.0f * scale);
	robotoBold20 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Bold.ttf", 20.0f * scale);
	robotoLight20 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Light.ttf", 20.0f * scale);
	robotoRegular42 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Regular.ttf", 42.0f * scale);
	robotoBold42 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Bold.ttf", 42.0f * scale);
	robotoLight42 = io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Light.ttf", 42.0f * scale);
	ImGui::GetStyle().ScaleAllSizes(scale);

	ImGui::StyleColorsDark();
	ImGui::GetStyle().FrameRounding = 5.0;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void GuiWindow::onTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
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

void GuiWindow::onRender(bool windowResized) {
	gui();
}

void GuiWindow::onResize(GLFWwindow* window, int w, int h) {
	glfwMakeContextCurrent(window);
	glfwPollEvents();
	onRenderBegin();
	onRender(true);
	onRenderEnd();
	glfwSwapBuffers(window);
}
