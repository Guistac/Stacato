#pragma once

struct GLFWwindow;

namespace ApplicationWindow {

	void init();
	void terminate();
	
	void open(int w, int h);
	void requestClose();
	void cancelCloseRequest();
	bool isCloseRequested();
	void close();

	void render();
	void onRender();

	void drawGui();
	void drawMainWindow();
	void drawMainMenuBar();
	void drawToolbar(float height);

	void quitApplicationModal();
	void aboutModal();

    float getScaleTuning();

	GLFWwindow* getGlfwWindow();

};


