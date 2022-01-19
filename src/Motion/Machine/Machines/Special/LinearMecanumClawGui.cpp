#include <pch.h>
#include "LinearMecanumClaw.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Motion/Axis/PositionControlledAxis.h"

void LinearMecanumClaw::controlsGui() {
	//Machine Controls Gui
}

void LinearMecanumClaw::settingsGui() {
	//Machine Settings Gui
}

void LinearMecanumClaw::axisGui() {
	//Gui for enumerating connected axis (actally useful ?)
}

void LinearMecanumClaw::deviceGui() {
	//Gui for enumerating connected devices (actually useful?)
}

void LinearMecanumClaw::metricsGui() {
	//Gui for metrics & performance graphs
}

float LinearMecanumClaw::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 15.0;
}

void LinearMecanumClaw::machineSpecificMiniatureGui() {
	
	float bottomControlsHeight = ImGui::GetTextLineHeight() * 4.4;
	float sliderHeight = ImGui::GetContentRegionAvail().y - bottomControlsHeight;
	float verticalWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 6.0) / 6;
	glm::vec2 verticalSliderSize(verticalWidgetWidth, sliderHeight);
	
	float linearAxisPositionProgress = 1.0;
	float linearAxisVelocityProgress = 1.0;
	const char* linearAxisPositionUnitShortFormString;
	float linearAxisMotionProgress = 0.0;
	float lienarAxisVelocityLimit = 0.0;
	static char linearAxisVelocityTargetString[32];
	static char linearAxisActualVelocityString[32];
	static char linearAxisActualPositionString[32];

	bool b_getValues = (isSimulating() && isSimulationReady()) || areHardwareNodesConnected();
	
	if (b_getValues) {
		
		linearAxisPositionProgress = getLinearAxisPositionProgress();
		linearAxisVelocityProgress = std::abs(getLinearAxisVelocityProgress());
		linearAxisPositionUnitShortFormString = Unit::getAbbreviatedString(getLinearAxis()->positionUnit);
		
		//motionProgress = targetInterpolation->getProgressAtTime(Environnement::getTime_seconds());
		//if(b_hasPositionTarget) positionTargetNormalized = (getManualPositionTarget() - minPosition) / (maxPosition - minPosition);
		//disableControls = !isEnabled();
		
		lienarAxisVelocityLimit = getLinearAxis()->getVelocityLimit_axisUnitsPerSecond();
		
		sprintf(linearAxisVelocityTargetString, "%.2f%s/s", linearAxisManualVelocityTarget, linearAxisPositionUnitShortFormString);
		
		if(isSimulating()){
			sprintf(linearAxisActualVelocityString, "%.2f%s/s", linearAxisMotionProfile.getVelocity(), linearAxisPositionUnitShortFormString);
			sprintf(linearAxisActualPositionString, "%.3f%s", linearAxisMotionProfile.getPosition(), linearAxisPositionUnitShortFormString);
		}else{
			//sprintf(linearAxisActualVelocityString, "%.2f%s/s", axisVelocityToMachineVelocity(getLinearAxis()->getActualVelocity_axisUnitsPerSecond()), positionUnitShortFormString);
			//sprintf(linearAxisActualPositionString, "%.3f%s", axisPositionToMachinePosition(getLinearAxis()->getActualPosition_axisUnits()), positionUnitShortFormString);
			sprintf(linearAxisActualVelocityString, "%.2f%s/s", getLinearAxis()->getActualVelocity_axisUnitsPerSecond(), linearAxisPositionUnitShortFormString);
			sprintf(linearAxisActualPositionString, "%.3f%s", getLinearAxis()->getActualPosition_axisUnits(), linearAxisPositionUnitShortFormString);
		}
	}
	else {
		sprintf(linearAxisVelocityTargetString, "-");
		sprintf(linearAxisActualVelocityString, "-");
		sprintf(linearAxisActualPositionString, "-");
		linearAxisPositionUnitShortFormString = "u";
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
	}

	//if(disableControls) BEGIN_DISABLE_IMGUI_ELEMENT

	float axisChildWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	
	ImGui::BeginChild("LINEAR", glm::vec2(axisChildWidth, ImGui::GetContentRegionAvail().y));
	
	ImGui::VSliderFloat("##ManualVelocity", verticalSliderSize, &linearAxisManualVelocityTarget, -lienarAxisVelocityLimit, lienarAxisVelocityLimit, "");
	if (ImGui::IsItemActive()) setLinearAxisVelocityTarget(linearAxisManualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setLinearAxisVelocityTarget(0.0);
	ImGui::SameLine();
	verticalProgressBar(linearAxisVelocityProgress, verticalSliderSize);
	ImGui::SameLine();
	verticalProgressBar(linearAxisPositionProgress, verticalSliderSize);

	/*
	if(b_hasPositionTarget){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float height = max.y - (max.y - min.y) * positionTargetNormalized;
		glm::vec2 lineStart(min.x, height);
		glm::vec2 lineEnd(max.x, height);
		ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, ImColor(Colors::white));
	}
	*/
		
	ImGui::PushFont(Fonts::robotoRegular12);
	glm::vec2 feedbackButtonSize(verticalSliderSize.x, ImGui::GetTextLineHeight());
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0, 0));
	ImGui::Button(linearAxisVelocityTargetString, feedbackButtonSize);
	ImGui::SameLine();
	ImGui::Button(linearAxisActualVelocityString, feedbackButtonSize);
	ImGui::SameLine();
	ImGui::Button(linearAxisActualPositionString, feedbackButtonSize);
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	ImGui::PopFont();

	float framePaddingX = ImGui::GetStyle().FramePadding.x;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(framePaddingX, ImGui::GetTextLineHeight() * 0.1));
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", linearAxisManualPositionTarget, linearAxisPositionUnitShortFormString);
	ImGui::InputFloat("##TargetPosition", &linearAxisManualPositionTarget, 0.0, 0.0, targetPositionString);
	linearAxisManualPositionTarget = std::min(linearAxisManualPositionTarget, (float)getLinearAxis()->getHighPositionLimit());
	linearAxisManualPositionTarget = std::max(linearAxisManualPositionTarget, (float)getLinearAxis()->getLowPositionLimit());
	if (linearAxisMotionProgress > 0.0 && linearAxisMotionProgress < 1.0) {
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * linearAxisMotionProgress, targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	ImGui::PopStyleVar();

	glm::vec2 doubleButtonSize((ImGui::GetContentRegionAvailWidth() - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 1.5);
	if (ImGui::Button("Move", doubleButtonSize)) moveLinearAxisToPosition(linearAxisManualVelocityTarget);
	ImGui::SameLine();
	if (ImGui::Button("Stop", doubleButtonSize)) moveLinearAxisToPosition(0.0);

	ImGui::EndChild();
	
	ImGui::SameLine();
	
	ImGui::BeginChild("CLAW", glm::vec2(axisChildWidth, ImGui::GetContentRegionAvail().y));
	
	
	
	
	
	ImGui::VSliderFloat("##ManualVelocity", verticalSliderSize, &linearAxisManualVelocityTarget, -lienarAxisVelocityLimit, lienarAxisVelocityLimit, "");
	if (ImGui::IsItemActive()) setLinearAxisVelocityTarget(linearAxisManualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setLinearAxisVelocityTarget(0.0);
	ImGui::SameLine();
	verticalProgressBar(linearAxisVelocityProgress, verticalSliderSize);
	ImGui::SameLine();
	verticalProgressBar(linearAxisPositionProgress, verticalSliderSize);
	
	ImGui::PushFont(Fonts::robotoRegular12);
	glm::vec2 feedbackButtonSize2(verticalSliderSize.x, ImGui::GetTextLineHeight());
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleColor(ImGuiCol_Button, Colors::darkGray);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(0, 0));
	ImGui::Button(linearAxisVelocityTargetString, feedbackButtonSize);
	ImGui::SameLine();
	ImGui::Button(linearAxisActualVelocityString, feedbackButtonSize);
	ImGui::SameLine();
	ImGui::Button(linearAxisActualPositionString, feedbackButtonSize);
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	ImGui::PopFont();

	float framePaddingX2 = ImGui::GetStyle().FramePadding.x;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(framePaddingX, ImGui::GetTextLineHeight() * 0.1));
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
	static char targetPositionString2[32];
	sprintf(targetPositionString, "%.3f %s", linearAxisManualPositionTarget, linearAxisPositionUnitShortFormString);
	ImGui::InputFloat("##TargetPosition", &linearAxisManualPositionTarget, 0.0, 0.0, targetPositionString);
	linearAxisManualPositionTarget = std::min(linearAxisManualPositionTarget, (float)getLinearAxis()->getHighPositionLimit());
	linearAxisManualPositionTarget = std::max(linearAxisManualPositionTarget, (float)getLinearAxis()->getLowPositionLimit());
	if (linearAxisMotionProgress > 0.0 && linearAxisMotionProgress < 1.0) {
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * linearAxisMotionProgress, targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	ImGui::PopStyleVar();

	glm::vec2 doubleButtonSize2((ImGui::GetContentRegionAvailWidth() - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 1.5);
	if (ImGui::Button("Move", doubleButtonSize)) moveLinearAxisToPosition(linearAxisManualVelocityTarget);
	ImGui::SameLine();
	if (ImGui::Button("Stop", doubleButtonSize)) moveLinearAxisToPosition(0.0);
	
	
	
	
	ImGui::EndChild();
	
	if (!b_getValues) {
		ImGui::PopStyleColor();
	}

	//if(disableControls) END_DISABLE_IMGUI_ELEMENT
	
	
}

