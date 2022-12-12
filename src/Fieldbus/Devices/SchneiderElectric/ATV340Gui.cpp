#include <pch.h>
#include "ATV340.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"

#include "Gui/Utilities/CustomWidgets.h"

void ATV340::deviceSpecificGui(){
	if(ImGui::BeginTabItem("ATV340")){
		if(ImGui::BeginTabBar("MicroFlexTabs")){
			if(ImGui::BeginTabItem("Controls")){
				controlTab();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Process Data")){
				processDataConfigTab();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Drive Configuration")){
				driveConfigTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}

void ATV340::controlTab(){
	
	ImVec2 statusBoxSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * .5, ImGui::GetTextLineHeight() * 2.0);
	
	if(motor->isEnabled()){
		if(ImGui::Button("Disable", statusBoxSize)) motor->disable();
	}else{
		ImGui::BeginDisabled(!motor->isReady());
		if(ImGui::Button("Enable", statusBoxSize)) motor->enable();
		ImGui::EndDisabled();
	}
	
	ImGui::SameLine();
	
	if(isOffline()) backgroundText("Offline", statusBoxSize, Colors::blue);
	else if(!isStateOperational()) backgroundText("Not Ready", statusBoxSize, Colors::red);
	else if(motor->isEmergencyStopped()) backgroundText("Safe Torque Off", statusBoxSize, Timing::getBlink(.5) ? Colors::red : Colors::yellow);
	else if(!axis->hasVoltage()) backgroundText("No Motor Voltage", statusBoxSize, Colors::orange);
	else if(axis->hasFault()) {
		static char faultString[256];
		sprintf(faultString, "Fault %x", lastFaultCode);
		backgroundText(faultString, statusBoxSize, Colors::red);
		if(ImGui::IsItemHovered()){
			ImGui::BeginTooltip();
			ImGui::Text("Fault %x : %s", lastFaultCode, getErrorCodeString());
			ImGui::EndTooltip();
		}
	}
	else if(motor->isEnabled()) backgroundText("Enabled", statusBoxSize, Colors::green);
	else if(motor->isReady()) backgroundText("Ready", statusBoxSize, Colors::yellow);
	else backgroundText("Not Ready", statusBoxSize, Colors::red);
	
	if(ImGui::Button("Fault Reset")) axis->doFaultReset();
	
	ImGui::SliderInt("##manualvel", &manualVelocityTarget_rpm, -velocityLimitRPM_Param->value, velocityLimitRPM_Param->value);
	if(ImGui::IsItemDeactivatedAfterEdit()) manualVelocityTarget_rpm = 0.0;
	
	ImGui::Separator();
	
	ImGui::Text("Fault: %i %s", axis->hasFault(), axis->hasFault() ? getErrorCodeString() : "No Fault");
	ImGui::Text("Voltage: %i", axis->hasVoltage());
	ImGui::Text("Warning: %i", axis->hasWarning());
	ImGui::Text("Remote Control: %i", axis->isRemoteControlActive());
	ImGui::Text("Internal Limit Reached: %i", axis->isInternalLimitReached());
	ImGui::Text("Target Reached: %i", axis->getOperatingModeSpeciricStatusWordBit_10());
	ImGui::Text("Power State Target: %s", Enumerator::getDisplayString(axis->getTargetPowerState()));
	ImGui::Text("Power State Actual: %s", Enumerator::getDisplayString(axis->getActualPowerState()));
}



void ATV340::processDataConfigTab(){
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Process Data Configuration");
	ImGui::PopFont();
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text("4 options or less can be selected.");
	ImGui::PopStyleColor();
	for(auto parameter : pdoConfigParameters.get()){
		parameter->gui();
		ImGui::SameLine();
		bool isEnabled = std::static_pointer_cast<BooleanParameter>(parameter)->value;
		if(isEnabled) ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", parameter->getName());
		if(isEnabled) ImGui::PopFont();
	}
}

void ATV340::driveConfigTab(){
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Drive Configuration");
	ImGui::PopFont();
	
	if(ImGui::Button("Upload Configuration")) configureDrive();
	ImGui::SameLine();
	if(ImGui::Button("Standstill Motor Tuning")) startMotorTuning();
	ImGui::SameLine();
	if(ImGui::Button("Restore Factory Settings")) resetFactorySettings();
	
	ImGui::Separator();
	auto drawParameterGroup = [](const char* groupName, ParameterGroup& group){
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader(groupName)){
			ImGui::PopFont();
			for(auto parameter : group.get()){
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%s", parameter->getName());
				ImGui::PopFont();
				parameter->gui();
			}
			ImGui::Spacing();
			ImGui::Separator();
		}else ImGui::PopFont();
	};
	
	drawParameterGroup("Motor Configuration", motorNameplateParameters);
	drawParameterGroup("Brake Logic", brakeLogicParameters);
	drawParameterGroup("Embedded Encoder", embeddedEncoderParameters);
	drawParameterGroup("Motor Control", motorControlParameters);
	drawParameterGroup("IO Configuration", ioConfigParameters);
}

