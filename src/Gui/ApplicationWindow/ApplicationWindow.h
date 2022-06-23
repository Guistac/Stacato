#pragma once

struct GLFWwindow;

namespace ApplicationWindow {

	void init();
	void terminate();
	
	void open();
	void update();

	void requestQuit();
	void quit();

	bool wasLaunchedByOpeningFile();
	const char* getOpenedFilePath();
	void onFileOpen(const char* filePath);

	void setWindowName(const char*);
	void showUnsavedModifications();
	void hideUnsavedModifications();

	double getMacOsTrackpadZoom();
	double getMacOsTrackpadRotate();
};


