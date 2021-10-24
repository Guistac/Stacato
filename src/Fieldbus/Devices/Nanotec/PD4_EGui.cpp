#include <pch.h>

#include "PD4_E.h"

#include <imgui.h>
#include <imgui_internal.h>

void PD4_E::deviceSpecificGui() {


    if (ImGui::BeginTabItem("PD4-E")) {

        ImGui::Text("Nanotec Drive Gui");

        
        ImGui::Text("Current Power State: %s", getDS402PowerState(powerState)->displayName);
        ImGui::Text("Requested Power State: %s", getDS402PowerState(requestedPowerState)->displayName);

		bool disablePowerControlButtons = !servoActuatorDevice->isReady();
		if(disablePowerControlButtons) BEGIN_DISABLE_IMGUI_ELEMENT
		if (servoActuatorDevice->isEnabled()) {
			if (ImGui::Button("Disable Operation")) requestedPowerState = DS402PowerState::State::READY_TO_SWITCH_ON;
		}
		else {
			if (ImGui::Button("Enable Operation")) requestedPowerState = DS402PowerState::State::OPERATION_ENABLED;
		}
		ImGui::SameLine();
		if (ImGui::Button("Quick Stop")) requestedPowerState = DS402PowerState::State::QUICKSTOP_ACTIVE;
		ImGui::SameLine();
		if(disablePowerControlButtons) END_DISABLE_IMGUI_ELEMENT
		if (ImGui::Button("Fault Reset")) performFaultReset = true;

		ImGui::Text("Operating Mode: %s", getOperatingMode(currentOperatingMode)->displayName);

		if (ImGui::BeginCombo("##opmode", getOperatingMode(requestedOperatingMode)->displayName)) {

			for (auto& mode : getOperatingModes()) {
				if (ImGui::Selectable(mode.displayName, requestedOperatingMode == mode.mode)) {
					requestedOperatingMode = mode.mode;
				}
			}

			ImGui::EndCombo();
		}
		float maxVelocity = 2000;
		float velocityControl = targetVelocity;
		ImGui::SliderFloat("Velocity", &velocityControl, -maxVelocity, maxVelocity, "%.1f rpm");
		targetVelocity = velocityControl;
		float velocityProgress = std::abs((float)actualVelocity / maxVelocity);
		ImGui::ProgressBar(velocityProgress);

        ImGui::EndTabItem();
    }
}

