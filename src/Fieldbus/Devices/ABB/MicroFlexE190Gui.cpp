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
	
	ImVec2 statusBoxSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * .5, ImGui::GetTextLineHeight() * 2.0);
	
	if(servo->isEnabled()){
		if(ImGui::Button("Disable", statusBoxSize)) servo->disable();
	}else{
		ImGui::BeginDisabled(!servo->isReady());
		if(ImGui::Button("Enable", statusBoxSize)) {
			servo->enable();
		}
		ImGui::EndDisabled();
	}
	
	ImGui::SameLine();
	
	if(isOffline()) backgroundText("Offline", statusBoxSize, Colors::blue);
	else if(!isStateOperational()) backgroundText("Not Ready", statusBoxSize, Colors::red);
	else if(servo->isEmergencyStopActive()) backgroundText("Safe Torque Off", statusBoxSize, Timing::getBlink(.5) ? Colors::red : Colors::yellow);
	else if(axis->hasFault()) {
		static char faultString[256];
		sprintf(faultString, "Fault : %x", axis->getErrorCode());
		backgroundText(faultString, statusBoxSize, Colors::red);
		if(ImGui::IsItemHovered()){
			ImGui::BeginTooltip();
			ImGui::Text("Fault %x : %s", axis->getErrorCode(), getErrorCodeString());
			ImGui::EndTooltip();
		}
	}
	else if(servo->isEnabled()) backgroundText("Enabled", statusBoxSize, Colors::green);
	else if(servo->isReady()) backgroundText("Ready", statusBoxSize, Colors::yellow);

	double maxVel = velocityLimit_parameter->value;
	char manualVelocityString[64];
	sprintf(manualVelocityString, "Velocity Target: %.2f rev/s", manualVelocityTarget);
	ImGui::SliderFloat("##vel", &manualVelocityTarget, -maxVel, maxVel, manualVelocityString);
	if(ImGui::IsItemDeactivatedAfterEdit()) manualVelocityTarget = 0.0;
	
	ImVec2 progressBarSize = ImGui::GetItemRectSize();
	
	double positionNormalized = std::fmod(servo->getPosition(), 1.0);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	ImGui::ProgressBar(positionNormalized, progressBarSize, "");
	ImGui::PopStyleColor();
	char positionString[64];
	sprintf(positionString, "Position: %.3f rev", servo->getPosition());
	ImVec2 textPos(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
				   ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(positionString, textPos, TextAlignement::LEFT_MIDDLE);
	
	double velNormalized = std::abs(servo->getVelocity()) / servo->getVelocityLimit();
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, servo->getVelocity() > 0.0 ? Colors::green : Colors::red);
	ImGui::ProgressBar(velNormalized, progressBarSize, "");
	ImGui::PopStyleColor();
	char velocityString[64];
	sprintf(velocityString, "Velocity: %.2f rev/s", servo->getVelocity());
	textPos = ImVec2(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
					 ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(velocityString, textPos, TextAlignement::LEFT_MIDDLE);
	
	float loadProgress = servo->getEffort();
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
	sprintf(loadString, "Load: %.1f%%", servo->getEffort() * 100.0);
	textPos = ImVec2(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
					 ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(loadString, textPos, TextAlignement::LEFT_MIDDLE);
	
	float followingErrorProgress = std::abs(actualPositionFollowingError) / maxFollowingError_parameter->value;
	char folErrString[64];
	sprintf(folErrString, "Following Error: %.3f rev", actualPositionFollowingError);
	ImGui::ProgressBar(followingErrorProgress, progressBarSize, "");
	textPos = ImVec2(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x,
					 ImGui::GetItemRectMin().y + ImGui::GetFrameHeight() * 0.5);
	textAligned(folErrString, textPos, TextAlignement::LEFT_MIDDLE);
	
	ImGui::Separator();
	
	ImGui::Text("Fault: %i %s", axis->hasFault(), getErrorCodeString());
	ImGui::Text("Voltage: %i", axis->hasVoltage());
	ImGui::Text("Warning: %i", axis->hasWarning());
	ImGui::Text("Remote Control: %i", axis->isRemoteControlActive());
	ImGui::Text("Internal Limit Reached: %i", axis->isInternalLimitReached());
	ImGui::Text("Power State Target: %s", Enumerator::getDisplayString(axis->getTargetPowerState()));
	ImGui::Text("Power State Actual: %s", Enumerator::getDisplayString(axis->getActualPowerState()));
	ImGui::Text("Operating Mode Target: %s", Enumerator::getDisplayString(axis->getOperatingModeTarget()));
	ImGui::Text("Operating Mode Actual: %s", Enumerator::getDisplayString(axis->getOperatingModeActual()));
	
	if(ImGui::Button("Reset Position")){
		servo->overridePosition(0.0);
	}
	
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
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Invert Direction");
	ImGui::PopFont();
	invertDirection_parameter->gui();
	ImGui::SameLine();
	ImGui::Text("Motor direction is%s inverted", invertDirection_parameter->value ? "" : " not");
	 
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Current Limit");
	ImGui::PopFont();
	currentLimit_parameter->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Max Following Error");
	ImGui::PopFont();
	maxFollowingError_parameter->gui();
	
	if(ImGui::Button("Upload Drive Configuration")) uploadDriveConfiguration();
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("Parameters should only be uploaded while the fieldbus is not running.");
	ImGui::PopStyleColor();
}
