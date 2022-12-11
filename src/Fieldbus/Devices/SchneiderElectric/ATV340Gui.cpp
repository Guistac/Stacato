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
	
	ImGui::SliderInt("##manualvel", &manualVelocityTarget_rpm, -velocityLimitRPM_param->value, velocityLimitRPM_param->value);
	if(ImGui::IsItemDeactivatedAfterEdit()) manualVelocityTarget_rpm = 0.0;
	
	
	/*
	double maxVel = velocityLimit_parameter->value;
	char manualVelocityString[64];
	sprintf(manualVelocityString, "Velocity Target: %.2f rev/s", manualVelocityTarget);
	ImGui::SliderFloat("##vel", &manualVelocityTarget, -maxVel, maxVel, manualVelocityString);
	if(ImGui::IsItemDeactivatedAfterEdit()) manualVelocityTarget = 0.0;
	
	ImVec2 progressBarSize = ImGui::GetItemRectSize();
	
	double positionNormalized = std::fmod(servo->position, 1.0);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	ImGui::ProgressBar(positionNormalized, progressBarSize, "");
	ImGui::PopStyleColor();
	char positionString[64];
	sprintf(positionString, "Position: %.3f rev", servo->position);
	ImVec2 textPos(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
				   ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(positionString, textPos, TextAlignement::LEFT_MIDDLE);
	
	double velNormalized = std::abs(servo->velocity) / velocityLimit_parameter->value;
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, servo->velocity > 0.0 ? Colors::green : Colors::red);
	ImGui::ProgressBar(velNormalized, progressBarSize, "");
	ImGui::PopStyleColor();
	char velocityString[64];
	sprintf(velocityString, "Velocity: %.2f rev/s", servo->velocity);
	textPos = ImVec2(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
					 ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(velocityString, textPos, TextAlignement::LEFT_MIDDLE);
	
	float loadProgress = servo->load;
	while(loadProgress > 1.0) loadProgress -= 1.0;
	if(*load_Value > 2.0) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::yellow);
	}else if(*load_Value > 1.0){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::green);
	}else{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_FrameBg));
	}
	ImGui::ProgressBar(loadProgress, progressBarSize, "");
	ImGui::PopStyleColor(2);
	char loadString[64];
	sprintf(loadString, "Load: %.1f%%", servo->load * 100.0);
	textPos = ImVec2(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
					 ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(loadString, textPos, TextAlignement::LEFT_MIDDLE);
	 */
	
	ImGui::Separator();
	
	ImGui::Text("Fault: %i %s", axis->hasFault(), getErrorCodeString());
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
	if(ImGui::Button("Upload Configuration")) configureMotor();
	ImGui::Separator();
	auto drawParameterGroup = [](const char* groupName, ParameterGroup& group){
		ImGui::PushFont(Fonts::sansBold15);
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
}

