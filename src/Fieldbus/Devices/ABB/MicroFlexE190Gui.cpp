#include <pch.h>

#include "MicroFlexE190.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/HelpMarker.h"


void MicroFlex_e190::deviceSpecificGui() {
	if(ImGui::BeginTabItem("MicroFlex e190")){
		controlTab();
		ImGui::EndTabItem();
	}
}

void MicroFlex_e190::controlTab(){

	if(b_isEnabled){
		if(ImGui::Button("Disable")) b_shouldDisable = true;
	}else{
		ImGui::BeginDisabled(!b_isReady);
		if(ImGui::Button("Enable")) b_shouldEnable = true;
		ImGui::EndDisabled();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!axis->hasFault());
	if(ImGui::Button("Fault Reset")) axis->doFaultReset();
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	if(isStateNone()) ImGui::Text("Offline");
	else if(!isStateOperational()) ImGui::Text("Not Ready");
	else if(b_estop) ImGui::Text("Safe Torque Off");
	else if(axis->hasFault()) ImGui::Text("Fault : %s", getErrorCodeString());
	else if(b_isReady) ImGui::Text("Ready");
	else if(b_isEnabled) ImGui::Text("Enabled");
	
	ImGui::SliderFloat("vel", &manualVelocity, -maxVelocity, maxVelocity);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		manualVelocity = 0.0;
	}
	ImGui::Text("pos: %.3f", position);
	ImGui::Text("vel: %.3f", velocity);
	
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
