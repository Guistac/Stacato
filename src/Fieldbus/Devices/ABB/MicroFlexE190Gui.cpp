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
	
	ImGui::Text("ready: %i", axis.isReady());
	ImGui::Text("enabled: %i", axis.isEnabled());
	ImGui::Text("quickstop: %i", axis.isQuickstop());
	ImGui::Text("warning: %i", axis.hasWarning());
	ImGui::Text("fault: %i", axis.hasFault());
	ImGui::Text("voltage: %i", axis.hasVoltage());
	
	if(ImGui::Button("Enable")) axis.enable();
	if(ImGui::Button("Disable")) axis.disable();
	if(ImGui::Button("Quickstop")) axis.doQuickstop();
	if(ImGui::Button("Fault Reset")) axis.doFaultReset();
	
	ImGui::SliderFloat("vel", &manualVelocity, -maxVelocity, maxVelocity);
	if(ImGui::IsItemDeactivatedAfterEdit()){
		manualVelocity = 0.0;
	}
	
	ImGui::Text("pos: %.3f", position);
	ImGui::Text("vel: %.3f", velocity);
	
	/*
	ImGui::Text("Actual Power State: %s", Enumerator::getDisplayString(actualPowerState));
	
	if(ImGui::BeginCombo("Requested Power State", Enumerator::getDisplayString(requestedPowerState))){
		for(auto& state : Enumerator::getTypes<DS402::PowerState>()){
			if(ImGui::Selectable(state.displayString, requestedPowerState == state.enumerator)){
				requestedPowerState = state.enumerator;
			}
		}
		ImGui::EndCombo();
	}
	
	ImGui::Text("Actual Operating Mode: %s", Enumerator::getDisplayString(actualOperatingMode));
	
	if(ImGui::BeginCombo("Requested Operating Mode", Enumerator::getDisplayString(requestedOperatingMode))){
		for(auto& mode : Enumerator::getTypes<DS402::OperatingMode>()){
			if(ImGui::Selectable(mode.displayString, requestedOperatingMode == mode.enumerator)){
				requestedOperatingMode = mode.enumerator;
			}
		}
		ImGui::EndCombo();
	}
	*/
	
}
