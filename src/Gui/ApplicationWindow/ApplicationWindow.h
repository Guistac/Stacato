#pragma once

struct GLFWwindow;

namespace ApplicationWindow {

	void init();
	void terminate();
	
	void open(int w, int h);
	void update();

	void requestQuit();
	void cancelQuitRequest();
	bool isQuitRequested();
	void quit();

	bool wasLaunchedByOpeningFile();
	const char* getOpenedFilePath();

	void setWindowName(const char*);

	double getMacOsTrackpadZoom();
	double getMacOsTrackpadRotate();
};


