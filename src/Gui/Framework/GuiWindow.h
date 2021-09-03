#pragma once

struct GLFWwindow;
struct ImFont;

class GuiWindow {
public:

	static void open(int w, int h);
	static void refresh();
	static void close();

	static void setShouldClose();

private:

	static void onInit();
	static void onRenderBegin();
	static void onRender(bool);
	static void onRenderEnd();
	static void onTerminate();

	static void onResize(GLFWwindow*, int, int);
	static void onClose(GLFWwindow*);

	static GLFWwindow* window;

	static bool b_shouldClose;

};