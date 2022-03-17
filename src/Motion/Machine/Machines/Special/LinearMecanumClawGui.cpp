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
	
	float singleWidgetWidth = ImGui::GetContentRegionAvail().x;
	float tripleWidgetWidth = (singleWidgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	glm::vec2 singleButtonSize(singleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	glm::vec2 doubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	glm::vec2 largeDoubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);
	glm::vec2 progressBarSize(singleWidgetWidth, ImGui::GetFrameHeight());
	
	bool disableManualControls = !isEnabled();
	if(disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT
		
		
	//================= LINEAR CONTROLS ==================
		
	ImGui::PushID("LinearControls");
		
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Linear Axis Controls");
	ImGui::PopFont();
	
	double linearVelocityLimit = 0.0;
	PositionUnit linearPositionUnit = PositionUnit::METER;
	if(isLinearAxisConnected()){
		auto linearAxis = getLinearAxis();
		linearVelocityLimit = linearAxis->getVelocityLimit();
		linearPositionUnit = linearAxis->getPositionUnit();
	}
	
	ImGui::Text("Manual Velocity:");
	ImGui::SetNextItemWidth(singleWidgetWidth);
	ImGui::SliderFloat("##manVel", &linearManualVelocityDisplay, -linearVelocityLimit, linearVelocityLimit);
	if (ImGui::IsItemActive()) setLinearVelocity(linearManualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		linearManualVelocityDisplay = 0.0;
		setLinearVelocity(0.0);
	}

	ImGui::Text("Manual Position Control");

	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char linearTargetPositionString[32];
	sprintf(linearTargetPositionString, "%.3f %s", linearPositionTargetDisplay, Unit::getAbbreviatedString(linearPositionUnit));
	ImGui::InputFloat("##TargetPosition", &linearPositionTargetDisplay, 0.0, 0.0, linearTargetPositionString);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char linearTargetVelocityString[32];
	sprintf(linearTargetVelocityString, "%.3f %s/s", linearVelocityTargetDisplay, Unit::getAbbreviatedString(linearPositionUnit));
	ImGui::InputFloat("##TargetVelocity", &linearVelocityTargetDisplay, 0.0, 0.0, linearTargetVelocityString);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputFloat("##TargetTime", &linearTimeTargetDisplay, 0.0, 0.0, "%.3f s");

	if (ImGui::Button("Fast Move", tripleButtonSize)) moveLinearToTargetWithVelocity(linearPositionTargetDisplay, linearVelocityLimit);

	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize)) moveLinearToTargetWithVelocity(linearPositionTargetDisplay, linearVelocityTargetDisplay);

	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize)) moveLinearToTargetInTime(linearPositionTargetDisplay, linearTimeTargetDisplay);

	if (ImGui::Button("Stop", doubleButtonSize)) setLinearVelocity(0.0);

	ImGui::SameLine();
	if (ImGui::Button("Fast Stop", doubleButtonSize)) fastStopLinear();
	
	
	double linearVelocityProgress = 1.0;
	static char linearVelocityString[256];
	ImVec4 linearVelocityColor = Colors::blue;
	
	double linearPositionProgress = 1.0;
	static char linearPositionString[256];
	ImVec4 linearPositionColor = Colors::blue;
	
	double linearFollowingErrorProgress = 1.0;
	static char linearFollowingErrorString[256];
	ImVec4 linearFollowingErrorColor = Colors::blue;
	
	double linearMovementTimeProgress = 1.0;
	static char linearMovementString[256];
	ImVec4 linearMovementColor = Colors::blue;
	
	if(isLinearAxisConnected() && isEnabled()){
		linearVelocityProgress = getLinearAxisVelocityProgress();
		sprintf(linearVelocityString, "Linear Velocity : %.3f %s/s", getLinearAxisVelocity(), Unit::getAbbreviatedString(linearPositionUnit));
		if(linearVelocityProgress <= 1.0) linearVelocityColor = Colors::green;
		else linearVelocityColor = Colors::red;
		
		linearPositionProgress = getLinearAxisPositionProgress();
		sprintf(linearPositionString, "Linear Position : %.3f %s", getLinearAxisPosition(), Unit::getAbbreviatedString(linearPositionUnit));
		if(linearPositionProgress <= 1.0 && linearPositionProgress >= 0.0) linearPositionColor = Colors::green;
		else linearPositionColor = Colors::red;
		
		linearFollowingErrorProgress = getLinearAxisFollowingErrorProgress();
		sprintf(linearFollowingErrorString, "Linear Following Error : %.3f %s", getLinearAxisFollowingError(), Unit::getAbbreviatedString(linearPositionUnit));
		if(linearFollowingErrorProgress <= 1.0) linearFollowingErrorColor = Colors::green;
		else linearFollowingErrorColor = Colors::red;
		
		if(hasLinearAxisTargetMovement()){
			linearMovementTimeProgress = getLinearAxisTargetMovementProgress();
			sprintf(linearMovementString, "%.1f s", getLinearAxisRemainingTargetMovementTime());
			linearMovementColor = Colors::green;
		}else{
			linearMovementTimeProgress = 1.0;
			sprintf(linearMovementString, "Linear Axis Movement : No Target Movement");
		}
	}else{
		sprintf(linearPositionString, "Linear Axis Position : Machine Disabled");
		sprintf(linearVelocityString, "Linear Axis Velocity : Machine Disabled");
		sprintf(linearMovementString, "Linear Axis Movement : Machine Disabled");
	}
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, linearVelocityColor);
	ImGui::ProgressBar(linearVelocityProgress, progressBarSize, linearVelocityString);
	ImGui::PopStyleColor();
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, linearPositionColor);
	ImGui::ProgressBar(linearPositionProgress, progressBarSize, linearPositionString);
	ImGui::PopStyleColor();
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, linearFollowingErrorColor);
	ImGui::ProgressBar(linearFollowingErrorProgress, progressBarSize, linearFollowingErrorString);
	ImGui::PopStyleColor();
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, linearMovementColor);
	ImGui::ProgressBar(linearMovementTimeProgress, progressBarSize, linearMovementString);
	ImGui::PopStyleColor();
	
	ImGui::PopID();
	
	
	
	
		
	//================= Claw Controls CONTROLS ==================
		
	ImGui::PushID("ClawControls");
		
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Claw Axis Controls");
	ImGui::PopFont();


	ImGui::Text("Manual Velocity:");
	ImGui::SetNextItemWidth(singleWidgetWidth);
	ImGui::SliderFloat("##manVel", &clawManualVelocityDisplay, -clawVelocityLimit, clawVelocityLimit);
	if (ImGui::IsItemActive()) setClawVelocity(clawManualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		clawManualVelocityDisplay = 0.0;
		setClawVelocity(0.0);
	}

	ImGui::Text("Manual Position Control");

	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char clawTargetPositionString[32];
	sprintf(clawTargetPositionString, "%.3f %s", clawPositionTargetDisplay, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputFloat("##TargetPosition", &clawPositionTargetDisplay, 0.0, 0.0, clawTargetPositionString);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char clawTargetVelocityString[32];
	sprintf(clawTargetVelocityString, "%.3f %s/s", clawTimeTargetDisplay, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputFloat("##TargetVelocity", &clawTimeTargetDisplay, 0.0, 0.0, clawTargetVelocityString);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputFloat("##TargetTime", &clawTimeTargetDisplay, 0.0, 0.0, "%.3f s");

	if (ImGui::Button("Fast Move", tripleButtonSize)) moveClawToTargetWithVelocity(clawPositionTargetDisplay, clawVelocityLimit);

	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize)) moveClawToTargetWithVelocity(clawPositionTargetDisplay, clawVelocityTargetDisplay);

	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize)) moveClawToTargetInTime(clawPositionTargetDisplay, clawTimeTargetDisplay);

	if (ImGui::Button("Stop", doubleButtonSize)) setClawVelocity(0.0);

	ImGui::SameLine();
	if (ImGui::Button("Fast Stop", doubleButtonSize)) fastStopClaw();

	double clawVelocityProgress = 1.0;
	static char clawVelocityString[256];
	ImVec4 clawVelocityColor = Colors::blue;
	
	double clawPositionProgress = 1.0;
	static char clawPositionString[256];
	ImVec4 clawPositionColor = Colors::blue;
	
	double clawFollowingErrorProgress = 1.0;
	double clawErrorTresholdProgress = 0.0;
	static char clawFollowingErrorString[256];
	ImVec4 clawFollowingErrorColor = Colors::blue;
	
	double clawMovementTimeProgress = 1.0;
	static char clawMovementString[256];
	ImVec4 clawMovementTimeColor = Colors::blue;
	
	if(isClawFeedbackConnected() && isEnabled()){
		clawVelocityProgress = getClawAxisVelocityProgress();
		sprintf(clawVelocityString, "Claw Velocity : %.3f %s/s", getClawAxisVelocity(), Unit::getAbbreviatedString(clawPositionUnit));
		if(clawVelocityProgress <= 1.0) clawVelocityColor = Colors::green;
		else clawVelocityColor = Colors::red;
		
		clawPositionProgress = getClawAxisPositionProgress();
		sprintf(clawPositionString, "Claw Position : %.3f %s", getClawAxisPosition(), Unit::getAbbreviatedString(clawPositionUnit));
		if(clawPositionProgress <= 1.0 && clawPositionProgress >= 0.0) clawPositionColor = Colors::green;
		else clawPositionColor = Colors::red;
		
		clawFollowingErrorProgress = getClawAxisFollowingErrorProgress();
		clawErrorTresholdProgress = std::abs(clawPositionErrorThreshold) / clawMaxPositionFollowingError;
		
		sprintf(clawFollowingErrorString, "Claw Position Error : %.3f %s", getClawAxisFollowingError(), Unit::getAbbreviatedString(clawPositionUnit));
		if(clawFollowingErrorProgress < clawErrorTresholdProgress) clawFollowingErrorColor = Colors::green;
		else if(clawFollowingErrorProgress <= clawMaxPositionFollowingError) clawFollowingErrorColor = Colors::yellow;
		else clawFollowingErrorColor = Colors::red;
		
		if(hasClawAxisTargetMovement()){
			clawMovementTimeProgress = getClawAxisTargetMovementProgress();
			sprintf(clawMovementString, "Claw Movement Time : %.1f s", getClawAxisRemainingTargetMovementTime());
			clawMovementTimeColor = Colors::green;
		}else{
			clawMovementTimeProgress = 1.0;
			sprintf(clawMovementString, "Claw Movement Time : No Target Movement");
		}
	}else{
		sprintf(clawVelocityString, "Claw Velocity: Machine Disabled");
		sprintf(clawPositionString, "Claw Position: Machine Disabled");
		sprintf(clawFollowingErrorString, "Claw Position Error: Machine Disabled");
		sprintf(clawMovementString, "Claw Movement Time: Machine Disabled");
	}
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, clawVelocityColor);
	ImGui::ProgressBar(clawVelocityProgress, progressBarSize, clawVelocityString);
	ImGui::PopStyleColor();
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, clawPositionColor);
	ImGui::ProgressBar(clawPositionProgress, progressBarSize, clawPositionString);
	ImGui::PopStyleColor();
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, clawFollowingErrorColor);
	ImGui::ProgressBar(clawFollowingErrorProgress, progressBarSize, clawFollowingErrorString);
	ImGui::PopStyleColor();
	
	if(clawErrorTresholdProgress > 0.0){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float width = min.x + (max.x - min.x) * clawErrorTresholdProgress;
		glm::vec2 lineStart(width, min.y);
		glm::vec2 lineEnd(width, max.y);
		ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, ImColor(Colors::transparentWhite));
	}
	
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, clawMovementTimeColor);
	ImGui::ProgressBar(clawMovementTimeProgress, progressBarSize, clawMovementString);
	ImGui::PopStyleColor();
	
	glm::vec4 clawClosedSignalColor;
	if(*clawClosedSignal) clawClosedSignalColor = Colors::green;
	else clawClosedSignalColor = Colors::darkGray;
	BackgroundText::draw("Claw Closed Signal", singleButtonSize, clawClosedSignalColor);
	
	ImGui::PopID();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Machine Homing");
	ImGui::PopFont();
	
	if(isHoming()){
		if(ImGui::Button("Stop Homing", singleButtonSize)){
			stopHoming();
		}
	}else{
		if(ImGui::Button("Start Homing", singleButtonSize)){
			startHoming();
		}
	}
	
	glm::vec4 homingColor;
	switch(homingStep){
		case ClawHomingStep::NOT_STARTED:
			homingColor = Colors::darkGray;
			break;
		case ClawHomingStep::HOMING_FAILED:
			homingColor = Colors::red;
			break;
		default:
			homingColor = Colors::green;
			break;
	}
	BackgroundText::draw(getHomingStateString(), singleButtonSize, homingColor);
	
	
	
	
	
	
	if(disableManualControls) END_DISABLE_IMGUI_ELEMENT
	
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
		ImGui::Text("%s (Position Unit : %s)", linearAxis->getName(), Enumerator::getDisplayString(linearAxis->getPositionUnit()));
		if(linearAxis->getPositionUnitType() != PositionUnitType::LINEAR){
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
		ImGui::Text("%s (Position Unit : %s)", clawAxis->getName(), Enumerator::getDisplayString(clawAxis->getPositionUnit()));
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
	}else {
		ImGui::TextColored(Colors::red, "Not Connected");
		return;
	}
	
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
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	//TODO: compute claw velocity and acceleration limited by mecanum geometry
	
	ImGui::Text("Claw Upper Position Limit:");
	static char maxClawPositionString[256];
	sprintf(maxClawPositionString, "%.1f %s", clawPositionLimit, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##maxposclaw", &clawPositionLimit, 0.0, 0.0, maxClawPositionString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Claw Velocity Limit:");
	static char maxClawVelocityString[256];
	sprintf(maxClawVelocityString, "%.1f %s/s", clawVelocityLimit, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##maxvelclaw", &clawVelocityLimit, 0.0, 0.0, maxClawVelocityString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Claw Acceleration Limit:");
	static char maxClawAccelerationString[256];
	sprintf(maxClawAccelerationString, "%.1f %s/s2", clawAccelerationLimit, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##maxClawAcc", &clawAccelerationLimit, 0.0, 0.0, maxClawAccelerationString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Claw Manual Acceleration :");
	static char manualClawAccString[256];
	sprintf(manualClawAccString, "%.1f %s/s2", clawManualAcceleration, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##manClawAcc", &clawManualAcceleration, 0.0, 0.0, manualClawAccString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Claw Homing Velocity");
	static char clawHomingVelocityString[256];
	sprintf(clawHomingVelocityString, "%.3f %s/s", clawHomingVelocity, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##clawhomvel", &clawHomingVelocity, 0.0, 0.0, clawHomingVelocityString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Claw Position Controller");
	ImGui::PopFont();
	
	ImGui::Text("Claw Position Loop Proportional Gain :");
	ImGui::InputDouble("##clawGainProp", &clawPositionLoopProportionalGain, 0.0, 0.0, "%.3f");
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Claw Max Position Following Error :");
	static char clawFollowingErrorLimitString[256];
	sprintf(clawFollowingErrorLimitString, "%.3f %s", clawMaxPositionFollowingError, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##maxfoler", &clawMaxPositionFollowingError, 0.0, 0.0, clawFollowingErrorLimitString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Claw Position Error Treshold :");
	static char clawPositionErrorTresholdString[256];
	sprintf(clawPositionErrorTresholdString, "%.3f %s", clawPositionErrorThreshold, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##clawErrThresh", &clawPositionErrorThreshold, 0.0, 0.0, clawPositionErrorTresholdString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	PositionUnit linearAxisPositionUnit = getLinearAxis()->getPositionUnit();

	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Mecanum Settings");
	ImGui::PopFont();
	
	ImGui::Text("Mecanum wheel distance drom claw pivot :");
	static char mecanumWheelPivotDistanceString[256];
	sprintf(mecanumWheelPivotDistanceString, "%.3f%s", mecanumWheelDistanceFromClawPivot, Unit::getAbbreviatedString(linearAxisPositionUnit));
	ImGui::InputDouble("##pivotDistance", &mecanumWheelDistanceFromClawPivot, 0.0, 0.0, mecanumWheelPivotDistanceString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Mecanum wheel pivot angle when claw is closed :");
	static char mecanumWheelClawPivotAngleString[256];
	sprintf(mecanumWheelClawPivotAngleString, "%.3f%s", mecanumWheelClawPivotRadiusAngleWhenClosed, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##pivotAngleClosed", &mecanumWheelClawPivotRadiusAngleWhenClosed, 0.0, 0.0, mecanumWheelClawPivotAngleString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Mecanum Wheel Circumference");
	static char mecanumWheelCircumferenceString[256];
	sprintf(mecanumWheelCircumferenceString, "%.3f%s", mecanumWheelCircumference, Unit::getAbbreviatedString(linearAxisPositionUnit));
	ImGui::InputDouble("##wheelCirc", &mecanumWheelCircumference, 0.0, 0.0, mecanumWheelCircumferenceString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Linear Axis Settings");
	ImGui::PopFont();
	
	auto linearAxis = getLinearAxis();
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text("Linear axis velocity limit is %.3f%s/s", linearAxis->getVelocityLimit(), Unit::getAbbreviatedString(linearAxis->getPositionUnit()));
	ImGui::Text("Linear axis acceration limit is %.3f%s/s2", linearAxis->getAccelerationLimit(), Unit::getAbbreviatedString(linearAxis->getPositionUnit()));
	ImGui::PopStyleColor();
	
	static char linearManAccString[16];
	sprintf(linearManAccString, "%.3f %s/s\xc2\xb2", linearAxisManualAcceleration, Unit::getAbbreviatedString(linearAxis->getPositionUnit()));
	ImGui::Text("Manual Acceleration");
	ImGui::InputDouble("##ManLinAcc", &linearAxisManualAcceleration, 0.0, 0.0, linearManAccString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Rapids");
	ImGui::PopFont();
	
	static char displayString[256];
	
	ImGui::Text("Linear Axis Rapid Velocity");
	sprintf(displayString, "%.3f %s/s", linearAxisRapidVelocity, Unit::getAbbreviatedString(getLinearAxisPositionUnit()));
	ImGui::InputDouble("##linearRapidVel", &linearAxisRapidVelocity, 0.0, 0.0, displayString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();

	ImGui::Text("Claw Axis Rapid Velocity");
	sprintf(displayString, "%.3f %s/s", clawRapidVelocity, Unit::getAbbreviatedString(clawPositionUnit));
	ImGui::InputDouble("##clawRapidVel", &clawRapidVelocity, 0.0, 0.0, displayString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
}

void LinearMecanumClaw::axisGui() {
	if(ImGui::BeginTabBar("##axistab")){
		if(isLinearAxisConnected()){
			auto linearAxis = getLinearAxis();
			if(ImGui::BeginTabItem(linearAxis->getName())){
				if(ImGui::BeginTabBar(linearAxis->getName())){
					linearAxis->nodeSpecificGui();
					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}
		}
		if(isClawAxisConnected()){
			auto clawAxis = getClawAxis();
			if(ImGui::BeginTabItem(clawAxis->getName())){
				if(ImGui::BeginTabBar(clawAxis->getName())){
					clawAxis->nodeSpecificGui();
					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
}

void LinearMecanumClaw::deviceGui() {
	//Gui for enumerating connected devices (actually useful?)
}

void LinearMecanumClaw::metricsGui() {
	//Gui for metrics & performance graphs
}

float LinearMecanumClaw::getMiniatureWidth() {
	return ImGui::GetTextLineHeight() * 18.0;
}

void LinearMecanumClaw::machineSpecificMiniatureGui() {
	
	glm::vec2 axisChildSize((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0,
							ImGui::GetContentRegionAvail().y);
	
	float bottomControlsHeight = ImGui::GetTextLineHeight() * 4.3;
	float sliderHeight = ImGui::GetContentRegionAvail().y - bottomControlsHeight;
	glm::vec2 titleButtonSize;
	glm::vec2 verticalSliderSize;
	
	bool b_disableControls = !isEnabled();
	if(b_disableControls) BEGIN_DISABLE_IMGUI_ELEMENT
	
	//============= LINEAR AXIS MINIATURE
	
	ImGui::BeginChild("LinearAxisControls", axisChildSize);
	
	titleButtonSize = glm::vec2(ImGui::GetContentRegionAvail().x,
								ImGui::GetTextLineHeight());

	verticalSliderSize = glm::vec2((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0) / 3.0,
								   sliderHeight);
	
	BackgroundText::draw("Translation", titleButtonSize, Colors::darkGray);
	
	//---Sliders
	float linearVelocityLimit = 0.0;
	if(isLinearAxisConnected()) linearVelocityLimit = getLinearAxis()->getVelocityLimit();
	ImGui::VSliderFloat("##LinearManualVelocity", verticalSliderSize, &linearManualVelocityDisplay, -linearVelocityLimit, linearVelocityLimit, "");
	if (ImGui::IsItemActive()) setLinearVelocity(linearManualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setLinearVelocity(0.0);
		linearManualVelocityDisplay = 0.0;
	}
	
	ImGui::SameLine();
	verticalProgressBar(getLinearAxisVelocityProgress(), verticalSliderSize);
	
	ImGui::SameLine();
	verticalProgressBar(getLinearAxisPositionProgress(), verticalSliderSize);
	
	if(hasLinearAxisTargetMovement()){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float height = max.y - (max.y - min.y) * getLinearAxisMovementTargetNormalized();
		glm::vec2 lineStart(min.x, height);
		glm::vec2 lineEnd(max.x, height);
		ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, ImColor(Colors::white));
	}
	
	//---Indicators
	glm::vec2 feedbackButtonSize(verticalSliderSize.x, ImGui::GetTextLineHeight());
	
	ImGui::PushFont(Fonts::robotoRegular12);
	
	static char manualVelocityString[64];
	sprintf(manualVelocityString, "%.3f%s/s", linearManualVelocityDisplay, Unit::getAbbreviatedString(getLinearAxisPositionUnit()));
	BackgroundText::draw(manualVelocityString, feedbackButtonSize, Colors::darkGray);
	
	ImGui::SameLine();
	static char velocityString[64];
	sprintf(velocityString, "%.3f%s/s", getLinearAxisVelocity(), Unit::getAbbreviatedString(getLinearAxisPositionUnit()));
	BackgroundText::draw(velocityString, feedbackButtonSize, Colors::darkGray);
	
	ImGui::SameLine();
	static char positionString[64];
	sprintf(positionString, "%.3f%s", getLinearAxisPosition(), Unit::getAbbreviatedString(getLinearAxisPositionUnit()));
	BackgroundText::draw(positionString, feedbackButtonSize, Colors::darkGray);

	ImGui::PopFont();
	
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetTextLineHeight() * 3.0);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", linearPositionTargetDisplay, Unit::getAbbreviatedString(getLinearAxisPositionUnit()));
	ImGui::InputFloat("##TargetPosition", &linearPositionTargetDisplay, 0.0, 0.0, targetPositionString);
	if(isLinearAxisConnected()){
		auto linearAxis = getLinearAxis();
		linearPositionTargetDisplay = std::min((double)linearPositionTargetDisplay, linearAxis->getHighPositionLimit());
		linearPositionTargetDisplay = std::max((double)linearPositionTargetDisplay, linearAxis->getLowPositionLimit());
	}
	
	
	if(hasLinearAxisTargetMovement()){
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * getLinearAxisTargetMovementProgress(), targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	
	ImGui::SameLine();
	glm::vec2 movementControlButtonSize(ImGui::GetContentRegionAvail().x,
										ImGui::GetFrameHeight());
	
	if(hasLinearAxisTargetMovement()){
		if(ImGui::Button("Stop##Linear", movementControlButtonSize)) setLinearVelocity(0.0);
	}else{
		if(ImGui::Button("Go##Linear", movementControlButtonSize)) moveLinearToTargetInTime(linearPositionTargetDisplay, 0.0);
	}
	
	ImGui::EndChild();

	
	
	
	//=============== CLAW AXIS MINIATURE
	
	ImGui::SameLine();
	ImGui::BeginChild("ClawAxisControls", axisChildSize);
	
	BackgroundText::draw("Ouverture", titleButtonSize, Colors::darkGray);
	
	//---Sliders
	ImGui::VSliderFloat("##ClawManualVelocity", verticalSliderSize, &clawManualVelocityDisplay, -clawVelocityLimit, clawVelocityLimit, "");
	if (ImGui::IsItemActive()) setClawVelocity(clawManualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setClawVelocity(0.0);
		clawManualVelocityDisplay = 0.0;
	}
	
	ImGui::SameLine();
	verticalProgressBar(getClawAxisVelocityProgress(), verticalSliderSize);
	
	ImGui::SameLine();
	verticalProgressBar(getClawAxisPositionProgress(), verticalSliderSize);
	
	if(hasClawAxisTargetMovement()){
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float height = max.y - (max.y - min.y) * getClawAxisMovementTargetNormalized();
		glm::vec2 lineStart(min.x, height);
		glm::vec2 lineEnd(max.x, height);
		ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, ImColor(Colors::white));
	}
	
	//---Indicators
	ImGui::PushFont(Fonts::robotoRegular12);
	
	sprintf(manualVelocityString, "%.3f%s/s", clawManualVelocityDisplay, Unit::getAbbreviatedString(getClawAxisPositionUnit()));
	BackgroundText::draw(manualVelocityString, feedbackButtonSize, Colors::darkGray);
	
	ImGui::SameLine();
	sprintf(velocityString, "%.3f%s/s", getClawAxisVelocity(), Unit::getAbbreviatedString(getClawAxisPositionUnit()));
	BackgroundText::draw(velocityString, feedbackButtonSize, Colors::darkGray);
	
	ImGui::SameLine();
	sprintf(positionString, "%.3f%s", getClawAxisPosition(), Unit::getAbbreviatedString(getClawAxisPositionUnit()));
	BackgroundText::draw(positionString, feedbackButtonSize, Colors::darkGray);

	ImGui::PopFont();
	
	//---Target Position
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetTextLineHeight() * 3.0);
	sprintf(targetPositionString, "%.3f %s", clawPositionTargetDisplay, Unit::getAbbreviatedString(getClawAxisPositionUnit()));
	ImGui::InputFloat("##TargetPosition", &clawPositionTargetDisplay, 0.0, 0.0, targetPositionString);
	clawPositionTargetDisplay = std::min(std::abs(clawPositionTargetDisplay), (float)clawPositionLimit);
	
	if(hasClawAxisTargetMovement()){
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * getClawAxisTargetMovementProgress(), targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	
	ImGui::SameLine();
	
	if(hasClawAxisTargetMovement()){
		if(ImGui::Button("Stop##Claw", movementControlButtonSize)) setClawVelocity(0.0);
	}else{
		if(ImGui::Button("Go##Claw", movementControlButtonSize)) moveClawToTargetInTime(clawPositionTargetDisplay, 0.0);
	}
	
	ImGui::EndChild();
	
	if(b_disableControls) END_DISABLE_IMGUI_ELEMENT
}

