#include <pch.h>

#include "GuiWindow.h"


#include <Magnum/Platform/GLContext.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Platform/GLContext.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <glfw/glfw3.h>

//#include <glad/glad.h>
//#define IMGUI_IMPL_OPENGL_LOADER_GLAD
//#include <backends/imgui_impl_opengl3.cpp>
//#include <backends/imgui_impl_glfw.cpp>

//#include "Gui/Utilities/FileDialog.h"
//#include "Gui/Gui.h"



GLFWwindow* GuiWindow::window;
bool GuiWindow::b_shouldClose = false;

void GuiWindow::init(){
    glfwInit();
	//FileDialog::init();
}

void GuiWindow::open(int w, int h) {
#ifdef MACOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif
	window = glfwCreateWindow(w, h, "Stacato", nullptr, nullptr);
	glfwSetWindowSizeCallback(window, onResize);
	glfwSetWindowCloseCallback(window, onClose);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	//gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	onInit();
}

void GuiWindow::refresh() {

	glfwMakeContextCurrent(window);

	using namespace Magnum;

	{

		//Create Magnum context in an isolated scope
		//Platform::GLContext ctx{ argc, argv };
		int argc = 0;
		const char** argv = nullptr;
		Platform::GLContext ctx{ argc, argv };

		
		// Setup the colored triangle
		using namespace Math::Literals;

		struct TriangleVertex {
			Vector2 position;
			Color3 color;
		};
		const TriangleVertex data[]{
			{{-0.5f, -0.5f}, 0xff0000_rgbf},    //Left vertex, red color
			{{ 0.5f, -0.5f}, 0x00ff00_rgbf},    //Right vertex, green color
			{{ 0.0f,  0.5f}, 0x0000ff_rgbf}     //Top vertex, blue color
		};

		GL::Buffer buffer;
		buffer.setData(data);
		

		GL::Mesh mesh;
		mesh.setPrimitive(GL::MeshPrimitive::Triangles)
			.setCount(3)
			.addVertexBuffer(buffer, 0,
				Shaders::VertexColorGL2D::Position{},
				Shaders::VertexColorGL2D::Color3{});

		Shaders::VertexColorGL2D shader;
		
		// Loop until the user closes the window
		while (!glfwWindowShouldClose(window)) {

			// Render here
			GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);
			shader.draw(mesh);

			// Swap front and back buffers
			glfwSwapBuffers(window);

			// Poll for and process events
			glfwPollEvents();
		}

	}



	/*
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
	*/
}

void GuiWindow::close() {
	onTerminate();
	glfwSetWindowShouldClose(window, GLFW_TRUE);
	//FileDialog::terminate();
	glfwTerminate();
}

void GuiWindow::onInit() {
	//IMGUI_CHECKVERSION();
	
	//ImGui::CreateContext();
	//ImPlot::CreateContext();
	//ImGuiNodeEditor::CreateContext();

	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);

    float scaleTuning = 1.0;
    
#ifdef WIN32
    scaleTuning = 1.25;
#endif
    
#ifdef MACOS
    scaleTuning = 0.75;
#endif
    
    float scale = xScale * scaleTuning;

	//Fonts::load(scale);

	//ImGui::GetStyle().ScaleAllSizes(scale);
     
	//ImGui::StyleColorsDark();
	//ImGui::GetStyle().FrameRounding = 5.0;

	//ImGui_ImplGlfw_InitForOpenGL(window, true);
	//ImGui_ImplOpenGL3_Init("#version 410 core");
}

void GuiWindow::onTerminate() {
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGuiNodeEditor::DestroyContext();
	//ImNodes::DestroyContext();
	//ImPlot::DestroyContext();
	//ImGui::DestroyContext();
}

void GuiWindow::onRenderBegin() {
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();
}

void GuiWindow::onRenderEnd() {
	//ImGui::Render();
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
	// Update and Render additional Platform Windows
	//if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
	//	ImGui::UpdatePlatformWindows();
	//	ImGui::RenderPlatformWindowsDefault();
	//}
}

void GuiWindow::onRender(bool windowCloseRequested) {
	//gui(windowCloseRequested);
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
