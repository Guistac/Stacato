#include <pch.h>

#include "MachineTemplate.h"

#include <imgui.h>


void MachineTemplate::controlsGui() {
	//Machine Controls Gui
}

void MachineTemplate::settingsGui() {
	//Machine Settings Gui
}

void MachineTemplate::axisGui() {
	//Gui for enumerating connected axis (actally useful ?)
}

void MachineTemplate::deviceGui() {
	//Gui for enumerating connected devices (actually useful?)
}

void MachineTemplate::metricsGui() {
	//Gui for metrics & performance graphs
}

float MachineTemplate::getMiniatureWidth() {
	//returns width of miniature control window
	return ImGui::GetTextLineHeight() * 8.0;
}

void MachineTemplate::machineSpecificMiniatureGui() {
	//Gui for Miniature Control Window
}
