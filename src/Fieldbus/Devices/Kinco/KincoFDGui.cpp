#include <pch.h>
#include "KincoFD.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void KincoFD::deviceSpecificGui() {
	if(ImGui::BeginTabItem("Kinco FD")){
		
		if(actuator->isEnabled()){
			if(ImGui::Button("Disable")) actuator->disable();
		}
		else{
			ImGui::BeginDisabled(!actuator->isReady());
			if(ImGui::Button("Enable")) actuator->enable();
			ImGui::EndDisabled();
		}
		
		ImGui::Text("Actual Power State: %s", DS402Axis::getPowerStateString(axis->getActualPowerState()).c_str());
		ImGui::Text("Requested Power State: %s", DS402Axis::getTargetPowerStateString(axis->getTargetPowerState()).c_str());
		
		ImGui::Text("Actual Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeActual()).c_str());
		ImGui::Text("Target Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeTarget()).c_str());
		
		
		float manualVelocityTarget = 0.0;
		if(ImGui::SliderFloat("Vel", &manualVelocityTarget, -std::abs(maxVelocity_parameter->value), maxVelocity_parameter->value)){
			actuator->setVelocityTarget(manualVelocityTarget);
		}
		else if(ImGui::IsItemDeactivatedAfterEdit()){
			actuator->setVelocityTarget(0.0);
		}
		float velocityNormalized = std::abs(actuator->getVelocity() / actuator->getVelocityLimit());
		
		ImGui::Text("Position: %.3f", actuator->getPosition());
		ImGui::ProgressBar(velocityNormalized);
		
		float currentNormalized = actuator->getEffort();
		ImGui::Text("Drive Effort");
		ImGui::ProgressBar(currentNormalized);
		
		float followingErrorNormalized = std::abs(actuator->getFollowingError() / actuator->getFollowingErrorLimit());
		ImGui::Text("Following Error");
		ImGui::ProgressBar(followingErrorNormalized);
		
		ImGui::Separator();
		
		ImGui::Text("opmode b10 %i", axis->getOperatingModeSpecificStatusWordBit_10());
		ImGui::Text("opmode b12 %i", axis->getOperatingModeSpecificStatusWordBit_12());
		ImGui::Text("opmode b13 %i", axis->getOperatingModeSpecificStatusWordBit_13());
		ImGui::Text("manufacturer b8 %i", axis->getManufacturerSpecificStatusWordBit_8());
		ImGui::Text("manufacturer b14 %i", axis->getManufacturerSpecificStatusWordBit_14());
		ImGui::Text("manufacturer b15 %i", axis->getManufacturerSpecificStatusWordBit_15());
		ImGui::Text("Voltage %i", axis->hasVoltage());
		ImGui::Text("Warning %i", axis->hasWarning());
		ImGui::Text("Remote Active %i", axis->isRemoteControlActive());
		ImGui::Text("Internal Limit Reached %i", axis->isInternalLimitReached());
		
		if(ImGui::Button("Start Homing")){
			actuator->overridePosition(0);
		}
		
		if(ImGui::Button("Start Autotuning")) startAutoTuning();
		
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Parameters")){
		
		maxVelocity_parameter->gui(Fonts::sansBold15);
		maxAcceleration_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		ImGui::Text("Drive current limit is %.2fA", driveCurrentLimit);
		maxCurrent_parameter->gui(Fonts::sansBold15);
		velocityFeedforward_parameter->gui(Fonts::sansBold15);
		maxFollowingError_parameter->gui(Fonts::sansBold15);
		followingErrorTimeout_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		brakingResistorResistance_parameter->gui(Fonts::sansBold15);
		brakingResistorPower_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		DIN1Function_parameter->gui(Fonts::sansBold15);
		DIN2Function_parameter->gui(Fonts::sansBold15);
		DIN3Function_parameter->gui(Fonts::sansBold15);
		DIN4Function_parameter->gui(Fonts::sansBold15);
		DIN5Function_parameter->gui(Fonts::sansBold15);
		DIN6Function_parameter->gui(Fonts::sansBold15);
		DIN7Function_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		if(ImGui::Button("Upload Configuration")) uploadConfiguration();
		
		ImGui::EndTabItem();
	}
}
