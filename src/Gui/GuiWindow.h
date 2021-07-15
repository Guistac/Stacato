#pragma once

struct GLFWwindow;
struct ImFont;

class GuiWindow {
public:

	static void open();
	static void refresh();
	static void close();

	static ImFont* robotoRegular15;
	static ImFont* robotoBold15;
	static ImFont* robotoLight15;

	static ImFont* robotoRegular20;
	static ImFont* robotoBold20;
	static ImFont* robotoLight20;

	static ImFont* robotoRegular42;
	static ImFont* robotoBold42;
	static ImFont* robotoLight42;

private:

	static void onInit();
	static void onRenderBegin();
	static void onRender(bool);
	static void onRenderEnd();
	static void onTerminate();

	static void onResize(GLFWwindow*, int, int);

	static GLFWwindow* window;
};