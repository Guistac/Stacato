#include <pch.h>

#include "ApplicationWindow.h"


#include <Magnum/Platform/GLContext.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Platform/GLContext.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include <glfw/glfw3.h>

//custom loader is FlextGL inside magnum engine "config.h" is a dummy include to satisfy imgui
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM "config.h"
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>

#include <implot.h>
#include "Gui/NodeEditor/NodeEditorGui.h"

#include "Gui/Utilities/FileDialog.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"


namespace ApplicationWindow {

	GLFWwindow* window;
	bool b_closeRequested = false;
	bool b_shouldClose = false;

	void init() {
		glfwInit();
		FileDialog::init();
	}

	void terminate() {
		FileDialog::terminate();
		glfwTerminate();
	}

	void open(int w, int h) {
#ifdef MACOS
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
		Logger::info("OpenGL Version {}", (const char*)glGetString(GL_VERSION));
		//active V-Sync
		glfwSwapInterval(1);

		//initialize gl context for magnum engine
		Magnum::Platform::GLContext magnumGlContext;

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
		float scaleTuning = 1.0;
#ifdef WIN32
		scaleTuning = 1.25;
#endif
#ifdef MACOS
		scaleTuning = 0.75;
#endif
		float scale = xScale * scaleTuning;
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
		onRender(false);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}




	void onRender(bool shouldClose) {

		drawGui();

		using namespace Magnum;
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

		GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);
		shader.draw(mesh);
	}

}