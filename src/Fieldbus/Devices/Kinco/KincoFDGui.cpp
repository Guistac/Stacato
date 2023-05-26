#include <pch.h>
#include "KincoFD.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void KincoFD::deviceSpecificGui() {
	if(ImGui::BeginTabItem("Kinco FD")){
		
		ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		if(actuator->isEnabled()){
			if(ImGui::Button("Disable", buttonSize)) actuator->disable();
		}
		else{
			ImGui::BeginDisabled(!actuator->isReady());
			if(ImGui::Button("Enable", buttonSize)) actuator->enable();
			ImGui::EndDisabled();
		}

		float manualVelocityTarget = 0.0;
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if(ImGui::SliderFloat("##Vel", &manualVelocityTarget, -std::abs(maxVelocity_parameter->value), maxVelocity_parameter->value)){
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
		
		ImGui::Text("Status: %s", actuator->getStatusString().c_str());
		ImGui::Text("Actual Power State: %s", DS402Axis::getPowerStateString(axis->getActualPowerState()).c_str());
		ImGui::Text("Requested Power State: %s", DS402Axis::getTargetPowerStateString(axis->getTargetPowerState()).c_str());
		ImGui::Text("Actual Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeActual()).c_str());
		ImGui::Text("Target Operating Mode: %s", DS402Axis::getOperatingModeString(axis->getOperatingModeTarget()).c_str());
		ImGui::Text("Voltage %i", axis->hasVoltage());
		ImGui::Text("Warning %i", axis->hasWarning());
		ImGui::Text("Remote Active %i", axis->isRemoteControlActive());
		ImGui::Text("Internal Limit Reached %i", axis->isInternalLimitReached());
		ImGui::Text("opmode b10 %i", axis->getOperatingModeSpecificStatusWordBit_10());
		ImGui::Text("opmode b12 %i", axis->getOperatingModeSpecificStatusWordBit_12());
		ImGui::Text("opmode b13 %i", axis->getOperatingModeSpecificStatusWordBit_13());
		ImGui::Text("man b8 %i", axis->getManufacturerSpecificStatusWordBit_8());
		ImGui::Text("man b14 %i", axis->getManufacturerSpecificStatusWordBit_14());
		ImGui::Text("man b15 %i", axis->getManufacturerSpecificStatusWordBit_15());
		
		ImGui::Separator();
		
		static int encoderPositionOverride = 0;
		ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 5.0);
		ImGui::InputInt("##PositionOverride", &encoderPositionOverride, 0, 0);
		ImGui::SameLine();
		if(ImGui::Button("Override Encoder Position")) actuator->overridePosition(encoderPositionOverride);
		
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
		invertDirectionOfMotion_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		brakingResistorResistance_parameter->gui(Fonts::sansBold15);
		brakingResistorPower_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		DIN1Function_parameter->gui(Fonts::sansBold15);
		DIN1Polarity_parameter->gui(Fonts::sansBold15);
		
		DIN2Function_parameter->gui(Fonts::sansBold15);
		DIN2Polarity_parameter->gui(Fonts::sansBold15);
		
		DIN3Function_parameter->gui(Fonts::sansBold15);
		DIN3Polarity_parameter->gui(Fonts::sansBold15);
		
		DIN4Function_parameter->gui(Fonts::sansBold15);
		DIN4Polarity_parameter->gui(Fonts::sansBold15);
		
		DIN5Function_parameter->gui(Fonts::sansBold15);
		DIN5Polarity_parameter->gui(Fonts::sansBold15);
		
		DIN6Function_parameter->gui(Fonts::sansBold15);
		DIN6Polarity_parameter->gui(Fonts::sansBold15);
		
		DIN7Function_parameter->gui(Fonts::sansBold15);
		DIN7Polarity_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		DOUT1Function_parameter->gui(Fonts::sansBold15);
		DOUT1Polarity_parameter->gui(Fonts::sansBold15);
		
		DOUT2Function_parameter->gui(Fonts::sansBold15);
		DOUT2Polarity_parameter->gui(Fonts::sansBold15);
		
		DOUT3Function_parameter->gui(Fonts::sansBold15);
		DOUT3Polarity_parameter->gui(Fonts::sansBold15);
		
		DOUT4Function_parameter->gui(Fonts::sansBold15);
		DOUT4Polarity_parameter->gui(Fonts::sansBold15);
		
		DOUT5Function_parameter->gui(Fonts::sansBold15);
		DOUT5Polarity_parameter->gui(Fonts::sansBold15);
		
		ImGui::Separator();
		
		if(ImGui::Button("Upload Configuration")) uploadConfiguration();
		
		ImGui::EndTabItem();
	}
}
