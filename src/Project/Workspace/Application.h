#pragma once

#include "Gui.h"
#include "Workspace.h"

namespace Application{

void setInitializationFunction(std::function<bool()> fn);
void setTerminationFunction(std::function<bool()> fn);

void run();

void requestQuit();
void quitImmediately();

float getTrackpadRotationDelta();
float getTrackpadZoomDelta();
	
};
