#include <pch.h>

#include "BinaryOscillator6x.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Colors.h"

#include "Motion/Axis/PositionControlledAxis.h"

#include "Utilities/CircularBuffer.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Gui/Assets/Fonts.h"

#include "Motion/SubDevice.h"
#include "NodeGraph/Device.h"

void BinaryOscillator6x::controlsGui() {

	bool b_disableAllControls = !isEnabled();

	if(b_disableAllControls) BEGIN_DISABLE_IMGUI_ELEMENT

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Device Status");
	ImGui::PopFont();

	int deviceCount = getGpioDeviceCount();
	for (int i = 0; i < deviceCount; i++) {
		std::shared_ptr<GpioDevice> device = getGpioDevice(i);

		static char deviceNameString[64];
		sprintf(deviceNameString, "%s on %s", device->getName(), device->parentDevice->getName());

		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button(deviceNameString);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (device->isReady()) {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			ImGui::Button("Enable");
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
			ImGui::Button("Not Ready");
		}
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();

	}

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Control");
	ImGui::PopFont();

	glm::vec2 tripleButtonSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 3.0, ImGui::GetTextLineHeight() * 2.0);
	static char manualButtonString[64];
	for (int i = 0; i < 6; i++) {
		if (i != 0 && i != 3) ImGui::SameLine();
		if (outputSignals[i]) {
			sprintf(manualButtonString, "Disable Output %i", i);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if (ImGui::Button(manualButtonString, tripleButtonSize)) {
				manuallySetOutput(i, false);
			}
		}
		else {
			sprintf(manualButtonString, "Enable Output %i", i);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
			if (ImGui::Button(manualButtonString, tripleButtonSize)) {
				manuallySetOutput(i, true);
			}
		}
		ImGui::PopStyleColor();
	}

	glm::vec2 doubleButtonSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);
	if (ImGui::Button("Enable All", doubleButtonSize)) {
		for (int i = 0; i < 6; i++) manuallySetOutput(i, true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Disable All", doubleButtonSize)) {
		for (int i = 0; i < 6; i++) manuallySetOutput(i, false);
	}

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Oscillator Control");
	ImGui::PopFont();

	ImGui::Text("Minimum Off-Time (seconds)");
	bool b_disableParameter = minOffTimeParameter->hasParameterTrack();
	if(b_disableParameter) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::SliderFloat("##minOff", &minOffTime_seconds, 1.0, maxTime_seconds, "%.1f s");
	minOffTime_seconds = std::max(1.0f, minOffTime_seconds);
	if(b_disableParameter) END_DISABLE_IMGUI_ELEMENT
	ImGui::Text("Maximum Off-Time (seconds)");
	b_disableParameter = maxOffTimeParameter->hasParameterTrack();
	if(b_disableParameter) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::SliderFloat("##maxOff", &maxOffTime_seconds, 1.0, maxTime_seconds, "%.1f s");
	maxOffTime_seconds = std::max(1.0f, maxOffTime_seconds);
	if(b_disableParameter) END_DISABLE_IMGUI_ELEMENT
	ImGui::Text("Minimum On-Time (seconds)");
	b_disableParameter = minOnTimeParameter->hasParameterTrack();
	if(b_disableParameter) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::SliderFloat("##minOn", &minOnTime_seconds, 1.0, maxTime_seconds, "%.1f s");
	minOnTime_seconds = std::max(1.0f, minOnTime_seconds);
	if(b_disableParameter) END_DISABLE_IMGUI_ELEMENT
	ImGui::Text("Maximum On-Time (seconds)");
	b_disableParameter = maxOnTimeParameter->hasParameterTrack();
	if(b_disableParameter) BEGIN_DISABLE_IMGUI_ELEMENT
	ImGui::SliderFloat("##maxOn", &maxOnTime_seconds, 1.0, maxTime_seconds, "%.1f s");
	maxOnTime_seconds = std::max(1.0f, maxOnTime_seconds);
	if(b_disableParameter) END_DISABLE_IMGUI_ELEMENT

	glm::vec2 singleButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 2.0);

	if (isOscillatorActive()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		if (ImGui::Button("Stop Oscillator", singleButtonSize)) {
			stopOscillator();
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		if (ImGui::Button("Start Oscillator", singleButtonSize)) {
			startOscillator();
		}
	}
	ImGui::PopStyleColor();

	if(b_disableAllControls) END_DISABLE_IMGUI_ELEMENT

}


void BinaryOscillator6x::settingsGui() {
	ImGui::Text("No Machine Specific Settings Available");
}



void BinaryOscillator6x::deviceGui() {
	int deviceCount = getGpioDeviceCount();
	if (deviceCount == 0) return;

	if (ImGui::BeginTabBar("DeviceTabBar")) {

		for (int i = 0; i < deviceCount; i++) {
			std::shared_ptr<Device> device = getGpioDevice(i)->parentDevice;

			if (ImGui::BeginTabItem(device->getName())) {
				
				ImGui::PushFont(Fonts::robotoBold20);
				ImGui::Text(device->getName());
				ImGui::PopFont();

				if (ImGui::BeginTabBar("DeviceTabBar")) {
					device->nodeSpecificGui();
					ImGui::EndTabBar();
				}

				ImGui::EndTabItem();
			}

		}

		ImGui::EndTabBar();
	}
}

void BinaryOscillator6x::axisGui() {
	ImGui::Text("No Axis in this machine");
}


void BinaryOscillator6x::metricsGui() {
	ImGui::Text("No Metrics Avaiable");
}


float BinaryOscillator6x::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 10.0;
}

void BinaryOscillator6x::machineSpecificMiniatureGui() {

	bool b_disableAllControls = !isEnabled();

	if(b_disableAllControls) BEGIN_DISABLE_IMGUI_ELEMENT

	glm::vec2 singleOutputButtonSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0));

	static char manualButtonString[64];
	for (int i = 0; i < 6; i++) {
		if (i != 0 && i != 2 && i != 4) ImGui::SameLine();
		if (outputSignals[i]) {
			sprintf(manualButtonString, "Disable %i", i);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			if (ImGui::Button(manualButtonString, singleOutputButtonSize)) {
				manuallySetOutput(i, false);
			}
		}
		else {
			sprintf(manualButtonString, "Enable %i", i);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
			if (ImGui::Button(manualButtonString, singleOutputButtonSize)) {
				manuallySetOutput(i, true);
			}
		}
		ImGui::PopStyleColor();
	}

	glm::vec2 largerButtonSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 2.0);

	if (ImGui::Button("Enable All", largerButtonSize)) {
		for (int i = 0; i < 6; i++) {
			manuallySetOutput(i, true);
		}
	}
	if (ImGui::Button("Disable All", largerButtonSize)) {
		for (int i = 0; i < 6; i++) {
			manuallySetOutput(i, false);
		}
	}


	if (isOscillatorActive()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		if (ImGui::Button("Stop Oscillator", ImGui::GetContentRegionAvail())) {
			stopOscillator();
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkYellow);
		if (ImGui::Button("Start Oscillator", ImGui::GetContentRegionAvail())) {
			startOscillator();
		}
	}
	ImGui::PopStyleColor();

	ImGui::PopStyleVar();

	if(b_disableAllControls) END_DISABLE_IMGUI_ELEMENT

}