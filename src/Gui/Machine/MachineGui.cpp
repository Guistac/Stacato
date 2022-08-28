#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Machine/Machine.h"
#include "Animation/AnimationConstraint.h"

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




glm::vec2 Machine::reserveSpaceForMachineHeaderGui(){
	glm::vec2 cursor = ImGui::GetCursorPos();
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Dummy(glm::vec2(ImGui::GetTextLineHeight()));
	ImGui::PopFont();
	return cursor;
}

void Machine::machineHeaderGui(glm::vec2 cursorPosition, float width){
	glm::vec2 initialCursorPos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(cursorPosition);
	ImGui::PushFont(Fonts::sansBold20);
	if(width <= 0.0) width = ImGui::CalcTextSize(getName()).x;
	centeredText(getName(), glm::vec2(width, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
	ImGui::SetCursorPos(initialCursorPos);
}


void Machine::machineHeaderGui(float width){
	ImGui::PushFont(Fonts::sansBold20);
	centeredText(getName(), glm::vec2(width, ImGui::GetTextLineHeight()));
	ImGui::PopFont();
}

void Machine::machineStateControlGui(float width){
		
	glm::vec2 buttonSize(width / 2.0, ImGui::GetTextLineHeight() * 2.0);
	float rounding = ImGui::GetStyle().FrameRounding;
	
	glm::vec4 statusColor;
	glm::vec4 statusTextColor;
	const char* statusString;
	ImFont* statusTextFont;
	
	if(isEmergencyStopped()){
		statusColor = fmod(Timing::getProgramTime_seconds(), .5) < .25 ? Colors::red : Colors::yellow;
		statusString = "E-STOP";
		statusTextColor = Colors::black;
		statusTextFont = Fonts::sansBold15;
	}else{
		switch(getState()){
			case MotionState::OFFLINE:
				statusColor = Colors::blue;
				statusString = "Offline";
				statusTextColor = Colors::white;
				statusTextFont = Fonts::sansRegular15;
				break;
			case MotionState::NOT_READY:
				statusColor = Colors::red;
				statusString = "Not Ready";
				statusTextColor = Colors::black;
				statusTextFont = Fonts::sansRegular15;
				break;
			case MotionState::READY:
				statusColor = Colors::yellow;
				statusString = "Ready";
				statusTextColor = Colors::black;
				statusTextFont = Fonts::sansRegular15;
				break;
			case MotionState::ENABLED:
				if(b_halted){
					statusColor = Colors::orange;
					statusString = "Halted";
					statusTextColor = Colors::black;
					statusTextFont = Fonts::sansRegular15;
				}else{
					statusColor = Colors::green;
					statusString = "Enabled";
					statusTextColor = Colors::black;
					statusTextFont = Fonts::sansRegular15;
				}
				break;
		}
	}
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::black);
	ImGui::PushStyleColor(ImGuiCol_Button, statusColor);
	ImGui::PushStyleColor(ImGuiCol_Text, statusTextColor);
	ImGui::PushFont(statusTextFont);
	backgroundText(statusString, buttonSize, statusColor, statusTextColor, ImDrawFlags_RoundCornersLeft);
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::Text("%s", getStatusString().c_str());
		ImGui::EndTooltip();
	}
	
	ImGui::SameLine(.0f, .0f);
	ImGui::BeginDisabled(state != MotionState::READY && state != MotionState::ENABLED);
	if (isEnabled()) {
		if(customRoundedButton("Disable", buttonSize, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight)) disable();
	}
	else {
		if(customRoundedButton("Enable", buttonSize, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight)) enable();
	}
	ImGui::EndDisabled();
}
