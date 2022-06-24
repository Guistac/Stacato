#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Machine/Machine.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void Machine::nodeSpecificGui() {

	if (ImGui::BeginTabItem("Controls")) {
		ImGui::BeginChild("Controls");
		stateControlGui();
		ImGui::Separator();
		controlsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		ImGui::BeginChild("Settings");
		generalSettingsGui();
		ImGui::Separator();
		settingsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Axis")) {
		ImGui::BeginChild("Axis");
		axisGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		ImGui::BeginChild("Devices");
		deviceGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		ImGui::BeginChild("Metrics");
		metricsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}

void Machine::stateControlGui() {
	glm::vec2 buttonSize;
	int buttonCount = 2;
	buttonSize.x = (ImGui::GetContentRegionAvail().x - (buttonCount - 1) * ImGui::GetStyle().ItemSpacing.x) / buttonCount;
	buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

	bool readyToEnable = isReady();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Machine Enabled", buttonSize);
	}
	else if (readyToEnable) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Machine Ready", buttonSize);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Machine Not Ready", buttonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();

	ImGui::SameLine();
	ImGui::BeginDisabled(!readyToEnable);
	if (isEnabled()) {
		if (ImGui::Button("Disable Machine", buttonSize)) disable();
	}
	else {
		if (ImGui::Button("Enable Machine", buttonSize)) enable();
	}
	ImGui::EndDisabled();
}

void Machine::generalSettingsGui() {
	ImGui::Text("Machine Name :");
	int nameBufferSize;
	char* nameBuffer = getNameBuffer(nameBufferSize);
	ImGui::InputText("##name", nameBuffer, nameBufferSize);

	ImGui::Text("Short Name :");
	ImGui::InputText("##shortName", shortName, 16);
}

void Machine::machineHeaderGui(float width){
	ImGui::PushFont(Fonts::sansBold20);
	centeredText(getName(), glm::vec2(width, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
}

void Machine::machineStateControlGui(float width){
		
	glm::vec2 buttonSize(width / 2.0, ImGui::GetTextLineHeight() * 2.0);
	float rounding = ImGui::GetStyle().FrameRounding;
	bool b_ready = isReady();
	
	glm::vec4 statusColor;
	const char* statusString;
	if (isEnabled()) {
		statusColor = Colors::green;
		statusString = "Enabled";
	}
	else if (b_ready) {
		statusColor = Colors::yellow;
		statusString = "Ready";
	}
	else {
		statusColor = Colors::red;
		statusString = "Not Ready";
	}
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
	ImGui::PushStyleColor(ImGuiCol_Button, statusColor);
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	customRoundedButton(statusString, buttonSize, rounding, ImDrawFlags_RoundCornersLeft);
	ImGui::PopItemFlag();
	ImGui::PopStyleColor(2);
	
	ImGui::SameLine(.0f, .0f);
	ImGui::BeginDisabled(!b_ready);
	if (isEnabled()) {
		if(customRoundedButton("Disable", buttonSize, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight)) disable();
	}
	else {
		if(customRoundedButton("Enable", buttonSize, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight)) enable();
	}
	ImGui::EndDisabled();
}
