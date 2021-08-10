#include "Gui.h"

#include <imgui.h>
#include <iostream>

#include "Fieldbus/EtherCatFieldbus.h"

void etherCatParameters() {

	ImGui::Text("These parameters become active the next time the fieldbus is started");
	ImGui::Separator();

	bool intervalEdited = false;
	bool frequencyEdited = false;
	float processInterval_milliseconds = EtherCatFieldbus::processInterval_milliseconds;
	float processFrequency_Hertz = 1000.0 / processInterval_milliseconds;
	float processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processDataTimeout_milliseconds;
	float stableClockThreshold_milliseconds = EtherCatFieldbus::clockStableThreshold_milliseconds;

	ImGui::Text("Process Interval");
	if (ImGui::InputFloat("##Process Interval", &processInterval_milliseconds, 0.1f, 1.0f, "%.1fms")) intervalEdited = true;
	ImGui::Text("Process Frequency");
	if (ImGui::InputFloat("##Process Frequency", &processFrequency_Hertz, 0.1f, 1.0f, "%.1fHz")) frequencyEdited = true;
	ImGui::Text("Process Data Timeout Delay");
	ImGui::InputFloat("##Process Data Timeout Delay", &processDataTimeoutDelay_milliseconds, 0.1f, 1.0f, "%.1fms");
	ImGui::Text("Clock Stabilisation Threshold");
	ImGui::InputFloat("##Clock Stabilisation Threshold", &stableClockThreshold_milliseconds, 0.01f, 0.1f, "%.2fms");
	
	if (frequencyEdited) EtherCatFieldbus::processInterval_milliseconds = 1000.0 / processFrequency_Hertz;
	else EtherCatFieldbus::processInterval_milliseconds = processInterval_milliseconds;

	if (processDataTimeoutDelay_milliseconds > EtherCatFieldbus::processInterval_milliseconds)
		processDataTimeoutDelay_milliseconds = EtherCatFieldbus::processInterval_milliseconds;

	EtherCatFieldbus::processDataTimeout_milliseconds = processDataTimeoutDelay_milliseconds;
	EtherCatFieldbus::clockStableThreshold_milliseconds = stableClockThreshold_milliseconds;
}