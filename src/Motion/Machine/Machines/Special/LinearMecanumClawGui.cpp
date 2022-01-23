#include <pch.h>
#include "LinearMecanumClaw.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/Axis/VelocityControlledAxis.h"
#include "Motion/SubDevice.h"
#include "NodeGraph/Device.h"

void LinearMecanumClaw::controlsGui() {
	//Machine Controls Gui
}

void LinearMecanumClaw::settingsGui() {
	
	//===== DEVICES
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Devices & Axis");
	ImGui::PopFont();
	
	ImGui::Text("Linear Axis:");
	ImGui::SameLine();
	if(isLinearAxisConnected()){
		auto linearAxis = getLinearAxis();
		ImGui::Text("%s (Position Unit : %s)", linearAxis->getName(), Enumerator::getDisplayString(linearAxis->positionUnit));
		if(linearAxis->positionUnitType != PositionUnitType::LINEAR){
			ImGui::TextColored(Colors::red, "Linear Axis does not have Linear Position Unit");
			return;
		}
	}else {
		ImGui::TextColored(Colors::red, "Not Connected");
		return;
	}
	
	ImGui::Text("Mecanum Axis:");
	ImGui::SameLine();
	if(isClawAxisConnected()){
		auto clawAxis = getClawAxis();
		ImGui::Text("%s (Position Unit : %s)", clawAxis->getName(), Enumerator::getDisplayString(clawAxis->positionUnit));
	}else {
		ImGui::TextColored(Colors::red, "Not Connected");
		return;
	}
	
	ImGui::Text("Claw Feedback Device:");
	ImGui::SameLine();
	if(isClawFeedbackConnected()){
		auto feedbackDevice = getClawFeedbackDevice();
		if(feedbackDevice->parentDevice) ImGui::Text("%s on %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
		else ImGui::Text("%s on Node %s", feedbackDevice->getName(), clawPositionFeedbackPin->getNode()->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	ImGui::Text("Claw Reference Device:");
	ImGui::SameLine();
	if(isClawReferenceConnected()){
		auto referenceDevice = getClawReferenceDevice();
		if(referenceDevice->parentDevice) ImGui::Text("%s on %s", referenceDevice->getName(), referenceDevice->parentDevice->getName());
		else ImGui::Text("%s on Node %s", referenceDevice->getName(), clawReferenceDevicePin->getNode()->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	//===== CLAW SETTINGS
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Claw Settings");
	ImGui::PopFont();
	
	ImGui::Text("Claw Position Unit");
	if(ImGui::BeginCombo("##unit", Enumerator::getDisplayString(clawPositionUnit))){
		for(auto& unit : Enumerator::getTypes<PositionUnit>()){
			if(!isAngularPositionUnit(unit.enumerator)) continue;
			if(ImGui::Selectable(Enumerator::getDisplayString(unit.enumerator), clawPositionUnit == unit.enumerator)){
				clawPositionUnit = unit.enumerator;
			}
		}
		ImGui::EndCombo();
	}
	
	ImGui::Text("Feedback %s per Claw %s",
				Enumerator::getDisplayString(getClawFeedbackDevice()->getPositionUnit()),
				Enumerator::getDisplayString(clawPositionUnit));
	ImGui::InputDouble("##conversionClaw", &clawFeedbackUnitsPerClawUnit);
	
	//TODO: compute claw velocity and acceleration limited by mecanum geometry
	
	ImGui::Text("Claw Velocity Limit:");
	static char maxClawVelocityString[256];
	sprintf(maxClawVelocityString, "%.1f %s/s", clawVelocityLimit, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##maxvelclaw", &clawVelocityLimit, 0.0, 0.0, maxClawVelocityString);
	
	ImGui::Text("Claw Acceleration Limit:");
	static char maxClawAccelerationString[256];
	sprintf(maxClawAccelerationString, "%.1f %s/s2", clawAccelerationLimit, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##maxClawAcc", &clawAccelerationLimit, 0.0, 0.0, maxClawAccelerationString);
	
	PositionUnit linearAxisPositionUnit = getLinearAxis()->positionUnit;

	ImGui::Text("Mecanum wheel distance drom claw pivot :");
	static char mecanumWheelPivotDistanceString[256];
	sprintf(mecanumWheelPivotDistanceString, "%.3f%s", mecanumWheelDistanceFromClawPivot, Unit::getAbbreviatedString(linearAxisPositionUnit));
	ImGui::InputDouble("##pivotDistance", &mecanumWheelDistanceFromClawPivot, 0.0, 0.0, mecanumWheelPivotDistanceString);
	
	ImGui::Text("Mecanum wheel pivot angle when claw is closed :");
	static char mecanumWheelClawPivotAngleString[256];
	sprintf(mecanumWheelClawPivotAngleString, "%.3f%s", mecanumWheelClawPivotRadiusAngleWhenClosed, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##pivotAngleClosed", &mecanumWheelClawPivotRadiusAngleWhenClosed, 0.0, 0.0, mecanumWheelClawPivotAngleString);
	
	ImGui::Text("Mecanum Wheel Circumference");
	static char mecanumWheelCircumferenceString[256];
	sprintf(mecanumWheelCircumferenceString, "%.3f%s", mecanumWheelCircumference, Unit::getAbbreviatedString(linearAxisPositionUnit));
	ImGui::InputDouble("##wheelCirc", &mecanumWheelCircumference, 0.0, 0.0, mecanumWheelCircumferenceString);
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Linear Axis Settings");
	ImGui::PopFont();
	
	auto linearAxis = getLinearAxis();
	
	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", linearAxis->getVelocityLimit_axisUnitsPerSecond(), Unit::getAbbreviatedString(linearAxis->positionUnit));
	ImGui::InputDouble("##VelLimit", &linearAxis->velocityLimit_axisUnitsPerSecond, 0.0, 0.0, velLimitString);
	
	static char accLimitString[16];
	sprintf(accLimitString, "%.3f %s/s\xc2\xb2", linearAxis->getAccelerationLimit_axisUnitsPerSecondSquared(), Unit::getAbbreviatedString(linearAxis->positionUnit));
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &linearAxis->accelerationLimit_axisUnitsPerSecondSquared, 0.0, 0.0, accLimitString);
	
	
	
	
	
	
	
	
	
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

