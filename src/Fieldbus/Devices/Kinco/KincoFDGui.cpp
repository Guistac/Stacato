#include <pch.h>
#include "KincoFD.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void KincoFD::deviceSpecificGui() {
	if(ImGui::BeginTabItem("Kinco FD")){
		
		
		if(ImGui::Button("Enable")) b_enable = true;
		if(ImGui::Button("Disable")) b_disable = true;
		
		ImGui::Text("Actual Power State: %s", DS402Axis::getPowerStateString(axis->getActualPowerState()).c_str());
		ImGui::Text("Requested Power State: %s", DS402Axis::getTargetPowerStateString(axis->getTargetPowerState()).c_str());
		
		ImGui::Text("Actual Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeActual()).c_str());
		ImGui::Text("Target Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeTarget()).c_str());
		
		ImGui::Checkbox("Mode Selection", &b_modeSelection);
		
		ImGui::BeginDisabled(b_modeSelection);
		ImGui::SliderFloat("Pos", &posTarget, -10.0, 10.0);
		ImGui::EndDisabled();
		
		ImGui::BeginDisabled(!b_modeSelection);
		ImGui::SliderFloat("Vel", &velTarget, -std::abs(maxVelocity_parameter->value), maxVelocity_parameter->value);
		ImGui::EndDisabled();
		
		ImGui::Text("Position: %.3f", posActual);
		ImGui::Text("Velocity: %.3f", velActual);
		
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
		
		float currentNormalized = std::abs(float(axis->getActualCurrent()) / incrementsPerAmpere) / maxCurrent_parameter->value;
		ImGui::Text("Drive Effort");
		ImGui::ProgressBar(currentNormalized);
		
		float followingErrorNormalized = std::abs(float(axis->getActualPositionFollowingError()) / incrementsPerRevolution) / maxFollowingError_parameter->value;
		ImGui::Text("Following Error");
		ImGui::ProgressBar(followingErrorNormalized);
		
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
