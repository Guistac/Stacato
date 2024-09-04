#pragma once

#include "Gui/Gui.h"
#include "Workspace.h"

namespace Application{

void setInitializationFunction(std::function<bool(std::filesystem::path)> fn);
void setTerminationFunction(std::function<bool()> fn);
void setQuitRequestFunction(std::function<bool()> fn);

void run(int argcount, const char ** args);

void requestQuit();
void quitImmediately();

float getTrackpadRotationDelta();
float getTrackpadZoomDelta();
	
};
