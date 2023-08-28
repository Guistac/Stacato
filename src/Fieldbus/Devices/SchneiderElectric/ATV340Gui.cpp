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
	else if(motor->isEmergencyStopActive()) backgroundText("Safe Torque Off", statusBoxSize, Timing::getBlink(.5) ? Colors::red : Colors::yellow);
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
	
	if(ImGui::SliderFloat("##manualvel", &manualVelocityTarget_rps, -motor->getVelocityLimit(), motor->getVelocityLimit())){
		motor->setVelocityTarget(manualVelocityTarget_rps);
	}
	else if(ImGui::IsItemDeactivatedAfterEdit()) {
		motor->setVelocityTarget(0.0);
		manualVelocityTarget_rps = 0.0;
	}
	
	ImGui::Separator();
	
	ImGui::Text("Fault: %i %s", axis->hasFault(), axis->hasFault() ? getErrorCodeString() : "No Fault");
	ImGui::Text("Voltage: %i", axis->hasVoltage());
	ImGui::Text("Warning: %i", axis->hasWarning());
	ImGui::Text("Remote Control: %i", axis->isRemoteControlActive());
	ImGui::Text("Internal Limit Reached: %i", axis->isInternalLimitReached());
	ImGui::Text("Target Reached: %i", axis->getOperatingModeSpecificStatusWordBit_10());
	ImGui::Text("Power State Target: %s", Enumerator::getDisplayString(axis->getTargetPowerState()));
	ImGui::Text("Power State Actual: %s", Enumerator::getDisplayString(axis->getActualPowerState()));
}


static void drawParameterGroup(const char* groupName, ParameterGroup& group){
	ImGui::PushFont(Fonts::sansBold20);
	if(ImGui::CollapsingHeader(groupName)){
		ImGui::PopFont();
		
		if(ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_RowBg)){
			
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Value");
			
			float frameHeight = ImGui::GetFrameHeight();
			ImGui::PushFont(Fonts::sansBold15);
			ImVec2 offset(ImGui::GetStyle().CellPadding.y, (frameHeight - ImGui::GetTextLineHeight()) / 2.0);
			ImGui::PopFont();
			
			for(auto parameter : group.get()){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(ImVec2(cursorPos.x + offset.x, cursorPos.y + offset.y));
				
				ImGui::BeginDisabled(parameter->isDisabled());
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%s", parameter->getName());
				ImGui::PopFont();
				ImGui::EndDisabled();
				
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().CellPadding.y);
				parameter->gui();
			}
			
			ImGui::EndTable();
		}
		ImGui::Spacing();
	}else ImGui::PopFont();
}


void ATV340::processDataConfigTab(){
	drawParameterGroup("Process Data Selection", pdoConfigParameters);
	drawParameterGroup("Digital Signal Inversion", digitalSignalInversionParameters);
}

void ATV340::driveConfigTab(){
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Drive Configuration");
	ImGui::PopFont();
	
	if(ImGui::Button("Upload Configuration")) configureDrive();
	if(ImGui::Button("Standstill Motor Tuning")) startStandardTuning();
	if(ImGui::Button("Rotation Motor Tuning")) startRotationTuning();
	if(ImGui::Button("Restore Factory Settings")) resetFactorySettings();
	
	ImGui::Separator();
	
	drawParameterGroup("Motor", motorNameplateParameters);
	drawParameterGroup("Brake Logic", brakeLogicParameters);
	drawParameterGroup("Embedded Encoder", embeddedEncoderParameters);
	drawParameterGroup("Motion Control", motorControlParameters);
	drawParameterGroup("Digital IO", digitalIoConfigParameters);
	drawParameterGroup("Analog IO", analogIoConfigParameters);
}

