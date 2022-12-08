#include <pch.h>

#include "MicroFlexE190.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"

#include "Gui/Utilities/CustomWidgets.h"


void MicroFlex_e190::deviceSpecificGui() {
	if(ImGui::BeginTabItem("MicroFlex e190")){
		if(ImGui::BeginTabBar("MicroFlexTabs")){
			if(ImGui::BeginTabItem("Controls")){
				controlTab();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Settings")){
				settingsTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void MicroFlex_e190::controlTab(){

	if(servo->isEnabled()){
		if(ImGui::Button("Disable")) servo->disable();
	}else{
		ImGui::BeginDisabled(!servo->isReady());
		if(ImGui::Button("Enable")) servo->enable();
		ImGui::EndDisabled();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!axis->hasFault());
	if(ImGui::Button("Fault Reset")) axis->doFaultReset();
	ImGui::EndDisabled();
	
	std::string stateString;
	if(isOffline()) stateString = "Offline";
	else if(!isStateOperational()) stateString = "Not Ready";
	else if(servo->isEmergencyStopped()) stateString = "Safe Torque Off";
	else if(axis->hasFault()) stateString = "Fault : " + std::string(getErrorCodeString());
	else if(servo->isEnabled()) stateString = "Enabled";
	else if(servo->isReady()) stateString = "Ready";
	ImGui::SameLine();
	backgroundText(stateString.c_str(), Colors::darkGray);

	ImVec2 progressBarSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
	double velNormalized = std::abs(servo->velocity) / velocityLimit_parameter->value;
	ImGui::Text("Velocity");
	char velocityString[64];
	sprintf(velocityString, "Vel: %.1frev/s", servo->velocity);
	ImGui::ProgressBar(velNormalized, progressBarSize, velocityString);
	
	double maxVel = velocityLimit_parameter->value;
	ImGui::SliderFloat("vel", &manualVelocityTarget, -maxVel, maxVel);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		manualVelocityTarget = 0.0;
	}
	
	float loadProgress = *load_Value;
	char loadString[64];
	sprintf(loadString, "Load: %.1f%%", servo->load * 100.0);
	ImGui::ProgressBar(loadProgress, progressBarSize);

	//ImGui::InputFloat("Velocity Target", &manualVelocityTarget);
	ImGui::Text("profiler position: %.3f", profiler_position);
	ImGui::Text("actual position: %.3f", *position_Value);
	ImGui::Text("profiler velocity: %.3f", profiler_velocity);
	ImGui::Text("actual velocity: %.3f", *velocity_Value);
	
	ImGui::Separator();
	
	ImGui::Text("Fault: %i %s", axis->hasFault(), getErrorCodeString());
	ImGui::Text("Voltage: %i", axis->hasVoltage());
	ImGui::Text("Warning: %i", axis->hasWarning());
	ImGui::Text("Remote Control: %i", axis->isRemoteControlActive());
	ImGui::Text("Internal Limit Reached: %i", axis->isInternalLimitReached());
	ImGui::Text("Quickstop: %i", axis->isQuickstopActive());
	ImGui::Text("Power State Target: %s", Enumerator::getDisplayString(axis->getTargetPowerState()));
	ImGui::Text("Power State Actual: %s", Enumerator::getDisplayString(axis->getActualPowerState()));
	ImGui::Text("Operating Mode Target: %s", Enumerator::getDisplayString(axis->getOperatingModeTarget()));
	ImGui::Text("Operating Mode Actual: %s", Enumerator::getDisplayString(axis->getOperatingModeActual()));
	
	
	
	
}

void MicroFlex_e190::settingsTab(){
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Velocity Limit");
	ImGui::PopFont();
	velocityLimit_parameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Acceleration Limit");
	ImGui::PopFont();
	accelerationLimit_parameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Invert Direction");
	ImGui::PopFont();
	invertMotor_parameter->gui();
	ImGui::SameLine();
	ImGui::Text("Motor direction is%s inverted", invertMotor_parameter->value ? "" : " not");
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Current Limit");
	ImGui::PopFont();
	currentLimit_parameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Max Following Error");
	ImGui::PopFont();
	maxFollowingError_parameter->gui();
	
}
