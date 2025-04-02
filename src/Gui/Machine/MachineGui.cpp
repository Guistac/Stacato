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
	if(ImGui::BeginTabItem("Animatable")){
		animatableGui();
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
	machineStateControlGui(ImGui::GetContentRegionAvail().x);
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
			case DeviceState::OFFLINE:
				statusColor = Colors::blue;
				statusString = "Offline";
				statusTextColor = Colors::white;
				statusTextFont = Fonts::sansRegular15;
				break;
			case DeviceState::NOT_READY:
				statusColor = Colors::red;
				statusString = "Not Ready";
				statusTextColor = Colors::black;
				statusTextFont = Fonts::sansRegular15;
				break;
			case DeviceState::READY:
				statusColor = Colors::yellow;
				statusString = "Ready";
				statusTextColor = Colors::black;
				statusTextFont = Fonts::sansRegular15;
				break;
			case DeviceState::ENABLING:
				statusColor = Colors::darkGreen;
				statusString = "Enabling...";
				statusTextColor = Colors::white;
				statusTextFont = Fonts::sansRegular15;
				break;
			case DeviceState::DISABLING:
				statusColor = Colors::darkYellow;
				statusString = "Disabling...";
				statusTextColor = Colors::white;
				statusTextFont = Fonts::sansRegular15;
				break;
			case DeviceState::ENABLED:
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
	ImGui::BeginDisabled(state != DeviceState::READY && state != DeviceState::ENABLED);
	if (isEnabled()) {
		if(customRoundedButton("Disable", buttonSize, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight)) disable();
	}
	else {
		if(customRoundedButton("Enable", buttonSize, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersRight)) enable();
	}
	ImGui::EndDisabled();
}

void Machine::animatableGui(){
	for(int i = 0; i < animatables.size(); i++){
		ImGui::PushID(i);
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", animatables[i]->getName());
		ImGui::PopFont();
		animatables[i]->settingsGui();
		ImGui::Separator();
		ImGui::PopID();
	}
}
