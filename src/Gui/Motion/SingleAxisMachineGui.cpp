#include <pch.h>

#include "Motion/Machine/SingleAxisMachine.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"
#include <implot.h>

#include "NodeGraph/Device.h"
#include "Gui/Framework/Colors.h"


void SingleAxisMachine::miniatureGui() {
	ImGui::Text("test");
}

void SingleAxisMachine::controlsGui() {

	//====================== AXIS MANUAL CONTROLS ==============================

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Machine Control");
	ImGui::PopFont();

	glm::vec2 buttonSize;
	int buttonCount = 2;
	buttonSize.x = (ImGui::GetContentRegionAvail().x - (buttonCount - 1) * ImGui::GetStyle().ItemSpacing.x) / buttonCount;
	buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

	bool readyToEnable = isReady();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Machine Enabled", buttonSize);
	}
	else if (readyToEnable) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Machine Ready", buttonSize);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Machine Not Ready", buttonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();

	ImGui::SameLine();
	if(!readyToEnable) BEGIN_DISABLE_IMGUI_ELEMENT
	if (isEnabled()) {
		if (ImGui::Button("Disable Machine", buttonSize)) disable();
	}
	else {
		if (ImGui::Button("Enable Machine", buttonSize)) enable();
	}
	if (!readyToEnable) END_DISABLE_IMGUI_ELEMENT

	ImGui::Separator();

	bool disableManualControls = !b_enabled;
	if(disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT

	//------------------- MASTER MANUAL ACCELERATION ------------------------

	ImGui::Text("Acceleration for manual controls :");
	static char accelerationString[32];
	sprintf(accelerationString, "%.3f %s/s2", manualControlAcceleration_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##TargetAcceleration", &manualControlAcceleration_machineUnitsPerSecond, 0.0, 0.0, accelerationString);
	clamp(manualControlAcceleration_machineUnitsPerSecond, 0.0, accelerationLimit_machineUnitsPerSecondSquared);
	ImGui::Separator();

	//------------------- VELOCITY CONTROLS ------------------------

	float widgetWidth = ImGui::GetContentRegionAvail().x;

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	ImGui::SetNextItemWidth(widgetWidth);
	static char velocityTargetString[32];
	sprintf(velocityTargetString, "%.3f %s/s", manualVelocityTarget_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	if (ImGui::SliderFloat("##Velocity", &manualVelocityTarget_machineUnitsPerSecond, -velocityLimit_machineUnitsPerSecond, velocityLimit_machineUnitsPerSecond, velocityTargetString)) {
		clamp(manualVelocityTarget_machineUnitsPerSecond, -velocityLimit_machineUnitsPerSecond, velocityLimit_machineUnitsPerSecond);
		setVelocity(manualVelocityTarget_machineUnitsPerSecond);
	}
	if (ImGui::Button("Stop##Velocity", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) {
		setVelocity(0.0);
	}

	ImGui::Separator();

	//------------------------- POSITION CONTROLS --------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Position Control");
	ImGui::PopFont();

	float tripleWidgetWidth = (widgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", targetPosition_machineUnits, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##TargetPosition", &targetPosition_machineUnits, 0.0, 0.0, targetPositionString);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetVelocityString[32];
	sprintf(targetVelocityString, "%.3f %s/s", targetVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##TargetVelocity", &targetVelocity_machineUnitsPerSecond, 0.0, 0.0, targetVelocityString);
	clamp(targetVelocity_machineUnitsPerSecond, 0.0, velocityLimit_machineUnitsPerSecond);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetTime", &targetTime_seconds, 0.0, 0.0, "%.3f s");
	if (ImGui::Button("Fast Move", tripleButtonSize)) {
		moveToPositionWithVelocity(targetPosition_machineUnits, velocityLimit_machineUnitsPerSecond, manualControlAcceleration_machineUnitsPerSecond);
	}
	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize)) {
		moveToPositionWithVelocity(targetPosition_machineUnits, targetVelocity_machineUnitsPerSecond, manualControlAcceleration_machineUnitsPerSecond);
	}
	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize)) {
		moveToPositionInTime(targetPosition_machineUnits, targetTime_seconds, manualControlAcceleration_machineUnitsPerSecond);
	}

	if (ImGui::Button("Stop##Target", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) {
		setVelocity(0.0);
	}

	ImGui::Separator();

	//-------------------------------- FEEDBACK --------------------------------

		
	if (std::abs(profileVelocity_machineUnitsPerSecond) == std::abs(velocityLimit_machineUnitsPerSecond)) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
	else if (std::abs(profileVelocity_machineUnitsPerSecond) > std::abs(velocityLimit_machineUnitsPerSecond)) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
	else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	static char velocityString[16];
	sprintf(velocityString, "%.2f %s/s", profileVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	float velocityProgress = std::abs(profileVelocity_machineUnitsPerSecond) / velocityLimit_machineUnitsPerSecond;
	ImGui::Text("Current Velocity :");
	ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), velocityString);
	ImGui::PopStyleColor();

	float targetProgress = targetProgress = MotionCurve::getMotionCurveProgress(currentProfilePointTime_seconds, targetCurveProfile);
	double movementSecondsLeft = 0.0;
	if (controlMode != ControlMode::POSITION_TARGET) {
		targetProgress = 1.0;
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
		movementSecondsLeft = targetCurveProfile.rampOutEndTime - currentProfilePointTime_seconds;
	}
	static char movementProgressChar[8];
	sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), movementProgressChar);
	ImGui::PopStyleColor();


	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice = nullptr;
	double range;
	static char rangeString[64];
	if (positionFeedbackDeviceLink->isConnected()) {
		positionFeedbackDevice = positionFeedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
		range = positionFeedbackDevice->getPositionInRange();
		if (!positionFeedbackDevice->isReady()) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
			range = 1.0;
			sprintf(rangeString, "Feedback device not ready");
		}
		else if (range < 1.0 && range > 0.0) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
			sprintf(rangeString, "%.3f%%", range * 100.0);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
			sprintf(rangeString, "Encoder Outside Working Range ! (%.3f%%)", range * 100.0);
			range = 1.0;
		}
	}
	else {
		range = 1.0;
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		sprintf(rangeString, "No Feedback Device Connected");
	}
	ImGui::Text("Encoder Position in Working Range :");
	ImGui::ProgressBar(range, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), rangeString);
	ImGui::PopStyleColor();

	ImGui::Separator();

	//-------------------------- HOMING CONTROLS ---------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Homing");
	ImGui::PopFont();

	switch (positionLimitType) {
	case PositionLimitType::Type::LOW_LIMIT_SIGNAL:
	case PositionLimitType::Type::HIGH_LIMIT_SIGNAL:
	case PositionLimitType::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
	case PositionLimitType::Type::REFERENCE_SIGNAL:
		if (ImGui::Button("Start Homing", tripleButtonSize)) {}
		ImGui::SameLine();
		if (ImGui::Button("Cancel Homing", tripleButtonSize)) {}
		break;
	case PositionLimitType::Type::FEEDBACK_REFERENCE:
	case PositionLimitType::Type::NO_LIMIT:
		if (ImGui::Button("Reset Position Feedback")) {}
		break;
	}

	if(disableManualControls) END_DISABLE_IMGUI_ELEMENT

}








void SingleAxisMachine::settingsGui() {

	//------------------ GENERAL MACHINE SETTINGS -------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();

	ImGui::Text("Movement Type :");
	if (ImGui::BeginCombo("##AxisUnitType", getPositionUnitType(machinePositionUnitType)->displayName)) {
		for (PositionUnitType& unitType : getPositionUnitTypes()) {
			if (ImGui::Selectable(unitType.displayName, machinePositionUnitType == unitType.type)) {
				machinePositionUnitType = unitType.type;
				//if the machine type is changed but the machine unit is of the wrong type
				//change the machine unit to the first correct type automatically
				if (getPositionUnit(machinePositionUnit)->type != unitType.type) {
					switch (unitType.type) {
					case PositionUnit::Type::ANGULAR:
						machinePositionUnit = getAngularPositionUnits().front().unit;
						positionLimitType = getAngularPositionLimitTypes().front().type;
						break;
					case PositionUnit::Type::LINEAR:
						machinePositionUnit = getLinearPositionUnits().front().unit;
						positionLimitType = getLinearPositionLimitTypes().front().type;
						break;
					}
				}
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Position Unit :");
	if (ImGui::BeginCombo("##AxisUnit", getPositionUnit(machinePositionUnit)->displayName)) {
		if (machinePositionUnitType == PositionUnit::Type::LINEAR) {
			for (PositionUnit& unit : getLinearPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, machinePositionUnit == unit.unit)) machinePositionUnit = unit.unit;
			}
		}
		else if (machinePositionUnitType == PositionUnit::Type::ANGULAR) {
			for (PositionUnit& unit : getAngularPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, machinePositionUnit == unit.unit)) machinePositionUnit = unit.unit;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Actuator Command Type :");
	if (ImGui::BeginCombo("##CommandType", getCommandType(commandType)->displayName)) {
		for (CommandType& command : getCommandTypes()) {
			if (ImGui::Selectable(command.displayName, commandType == command.type)) commandType = command.type;
		}
		ImGui::EndCombo();
	}
	switch (commandType) {
		case CommandType::Type::POSITION_COMMAND: break;
		case CommandType::Type::VELOCITY_COMMAND:
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("Actuators with velocity command are not yet supported. In Closed Loop Control, the machine object would need a PID controller with position feedback to regulate position.");
			ImGui::PopStyleColor();
			break;
	}

	ImGui::Text("Motion Control Type :");
	if (ImGui::BeginCombo("##MotionControlType", getMotionControlType(motionControlType)->displayName)) {
		for (MotionControlType& control : getMotionControlTypes()) {
			if (ImGui::Selectable(control.displayName, motionControlType == control.type)) motionControlType = control.type;
		}
		ImGui::EndCombo();
	}
	switch (motionControlType) {
		case MotionControlType::Type::CLOSED_LOOP_CONTROL: break;
		case MotionControlType::Type::OPEN_LOOP_CONTROL:
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("Open Loop Controlled Machines are not yet Supported. In this control mode, no position feedback is used. Only use when there is no need for position control.");
			ImGui::PopStyleColor();
			break;
	}

	ImGui::Separator();


	//---------------------- POSITION FEEDBACK ---------------------------

	if (motionControlType == MotionControlType::Type::CLOSED_LOOP_CONTROL) {

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback");
		ImGui::PopFont();

		if (isPositionFeedbackDeviceConnected()) {
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getPositionFeedbackDevice();
			PositionFeedback* feedbackType = getPositionFeedbackType(feedbackDevice->feedbackType);
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Device:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s on %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Type:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s", feedbackType->displayName);

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Position Unit:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s", getPositionUnit(feedbackDevice->positionUnit)->displayNamePlural);

			switch (feedbackDevice->feedbackType) {
			case PositionFeedback::Type::INCREMENTAL_FEEDBACK:
				ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
				ImGui::TextWrapped("With incremental position feedback, the homing routine needs to be executed on each system power cycle");
				ImGui::PopStyleColor();
				break;
			case PositionFeedback::Type::ABSOLUTE_FEEDBACK: break;
			}
			ImGui::Text("%s %s per Machine %s :", feedbackDevice->getName(), getPositionUnit(feedbackDevice->positionUnit)->displayNamePlural, getPositionUnit(machinePositionUnit)->displayName);

			if(feedbackAndActuatorConversionIdentical) BEGIN_DISABLE_IMGUI_ELEMENT
			ImGui::InputDouble("##feedbackCoupling", &feedbackUnitsPerMachineUnits);
			if(feedbackAndActuatorConversionIdentical) END_DISABLE_IMGUI_ELEMENT
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Feedback device connected.");
			ImGui::PopStyleColor();
		}
		ImGui::Separator();
	}

	//---------------------- ACTUATOR -------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Actuator");
	ImGui::PopFont();

	if (isActuatorDeviceConnected()) {
		std::shared_ptr<ActuatorDevice> actuators = getActuatorDevice();
		CommandType* actuatorCommandType = getCommandType(actuators->commandType);

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Device:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s on %s", actuators->getName(), actuators->parentDevice->getName());

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Command Type:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", actuatorCommandType->displayName);

		if (actuatorCommandType->type != commandType) {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("Actuator Command Type doesn't match Machine command Type");
			ImGui::PopStyleColor();
		}

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Position Unit:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", getPositionUnit(actuators->positionUnit)->displayNamePlural);

		ImGui::Text("%s %s per Machine %s :", actuators->getName(), getPositionUnit(actuators->positionUnit)->displayNamePlural, getPositionUnit(machinePositionUnit)->displayName);
		ImGui::InputDouble("##actuatorCoupling", &actuatorUnitsPerMachineUnits);

		ImGui::Checkbox("##actFeedIdent", &feedbackAndActuatorConversionIdentical);
		ImGui::SameLine();
		ImGui::Text("Actuator and Feedback Coupling are identical");
		if (feedbackAndActuatorConversionIdentical) {
			feedbackUnitsPerMachineUnits = actuatorUnitsPerMachineUnits;
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("No Actuator device connected.");
		ImGui::PopStyleColor();
	}
	
	ImGui::Separator();

	//-------------------------- KINEMATIC LIMITS ----------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Kinematic Limits");
	ImGui::PopFont();

	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", velocityLimit_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##VelLimit", &velocityLimit_machineUnitsPerSecond, 0.0, 0.0, velLimitString);
	static char accLimitString[16];
	sprintf(accLimitString, "%.3f %s/s2", accelerationLimit_machineUnitsPerSecondSquared, getPositionUnitStringShort(machinePositionUnit));
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &accelerationLimit_machineUnitsPerSecondSquared, 0.0, 0.0, accLimitString);

	double halfWidgetWidth = (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;

	ImGui::Text("Default Manual Control Parameters", getPositionUnitStringPlural(machinePositionUnit));
	ImGui::SetNextItemWidth(halfWidgetWidth);
	static char manAccString[16];
	sprintf(manAccString, "%.3f %s/s2", defaultManualAcceleration_machineUnitsPerSecondSquared, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##defmanAcc", &defaultManualAcceleration_machineUnitsPerSecondSquared, 0.0, 0.0, manAccString);
	clamp(defaultManualAcceleration_machineUnitsPerSecondSquared, 0.0, accelerationLimit_machineUnitsPerSecondSquared);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(halfWidgetWidth);
	static char manVelString[16];
	sprintf(manVelString, "%.3f %s/s", defaultManualVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##defmanvel", &defaultManualVelocity_machineUnitsPerSecond, 0.0, 0.0, manVelString);
	clamp(defaultManualVelocity_machineUnitsPerSecond, 0.0, velocityLimit_machineUnitsPerSecond);

	ImGui::Separator();

	//----------------- POSITION REFERENCES AND HOMING -----------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Limits");
	ImGui::PopFont();

	ImGui::Text("Limit Type");
	if (ImGui::BeginCombo("##PositionReference", getPositionLimitType(positionLimitType)->displayName)) {
		switch(machinePositionUnitType){
		case PositionUnit::Type::LINEAR:
			for (PositionLimitType& reference : getLinearPositionLimitTypes()) {
				bool selected = positionLimitType == reference.type;
				if (ImGui::Selectable(reference.displayName, selected)) positionLimitType = reference.type;
			}
			break;
		case PositionUnit::Type::ANGULAR:
			for (PositionLimitType& reference : getAngularPositionLimitTypes()) {
				bool selected = positionLimitType == reference.type;
				if (ImGui::Selectable(reference.displayName, selected)) positionLimitType = reference.type;
			}
			break;
		}
		ImGui::EndCombo();
	}

	static char negDevString[16];
	sprintf(negDevString, "%.3f %s", allowedNegativeDeviationFromReference_machineUnits, getPositionUnitStringShort(machinePositionUnit));
	static char posDevString[16];
	sprintf(posDevString, "%.3f %s", allowedPositiveDeviationFromReference_machineUnits, getPositionUnitStringShort(machinePositionUnit));
	static char homVelString[16];
	sprintf(homVelString, "%.3f %s/s", homingVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));


	switch (positionLimitType) {
	case PositionLimitType::Type::LOW_LIMIT_SIGNAL:
		ImGui::Text("Max Deviation From Low Limit");
		ImGui::InputDouble("##MaxDeviation", &allowedPositiveDeviationFromReference_machineUnits, 0.0, 0.0, posDevString);
		if (allowedPositiveDeviationFromReference_machineUnits < 0.0) allowedPositiveDeviationFromReference_machineUnits = 0.0;
		ImGui::Text("Homing Velocity", getPositionUnit(machinePositionUnit)->displayNamePlural);
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
		if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
		ImGui::TextWrapped("Single Limit Signal at the negative end of the machine travel."
			"\nHoming will move the machine in the negative direction");
		break;
	case PositionLimitType::Type::HIGH_LIMIT_SIGNAL:
		ImGui::Text("Max Deviation From High Limit", getPositionUnit(machinePositionUnit)->displayNamePlural);
		ImGui::InputDouble("##MaxDeviation", &allowedNegativeDeviationFromReference_machineUnits, 0.0, 0.0, negDevString);
		if (allowedNegativeDeviationFromReference_machineUnits > 0.0) allowedNegativeDeviationFromReference_machineUnits = 0.0;
		ImGui::Text("Homing Velocity");
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
		if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
		ImGui::TextWrapped("Single Limit Signal at the positive end of the machine travel."
			"\nHoming will move the machine in the position direction");
		break;
	case PositionLimitType::Type::LOW_AND_HIGH_LIMIT_SIGNALS:
		ImGui::Text("Homing Direction");
		if (ImGui::BeginCombo("##HomingDirection", getHomingDirectionType(homingDirectionType)->displayName)) {
			for (HomingDirection& direction : getHomingDirectionTypes()) {
				bool selected = homingDirectionType == direction.type;
				if (ImGui::Selectable(direction.displayName, selected)) homingDirectionType = direction.type;
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Homing Velocity", getPositionUnit(machinePositionUnit)->displayNamePlural);
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
		if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
		ImGui::TextWrapped("Two Limit Signals at each end of the machine travel."
			"\nHoming will first move the machine in the specified direction, then the other direction");
		break;
	case PositionLimitType::Type::REFERENCE_SIGNAL:
		ImGui::Text("Homing Direction");
		if (ImGui::BeginCombo("##HomingDirection", getHomingDirectionType(homingDirectionType)->displayName)) {
			for (HomingDirection& direction : getHomingDirectionTypes()) {
				bool selected = homingDirectionType == direction.type;
				if (ImGui::Selectable(direction.displayName, selected)) homingDirectionType = direction.type;
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Max Positive Deviation");
		ImGui::InputDouble("##MaxPositiveDeviation", &allowedPositiveDeviationFromReference_machineUnits, 0.0, 0.0, posDevString);
		if (allowedPositiveDeviationFromReference_machineUnits < 0.0) allowedPositiveDeviationFromReference_machineUnits = 0.0;
		ImGui::Text("Max Negative Deviation");
		ImGui::InputDouble("##MaxNegativeDeviation", &allowedNegativeDeviationFromReference_machineUnits, 0.0, 0.0, negDevString);
		if (allowedNegativeDeviationFromReference_machineUnits > 0.0) allowedNegativeDeviationFromReference_machineUnits = 0.0;
		ImGui::Text("Homing Velocity", getPositionUnit(machinePositionUnit)->displayNamePlural);
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
		if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
		ImGui::TextWrapped("Single Limit Signal inside the machine travel range."
			"\nHoming will find the position reference using the specified direction."
			"\nThe machine will not go over the max deviations from the position reference. (Not recommended for machine with physical limits)");
		break;
	case PositionLimitType::Type::FEEDBACK_REFERENCE:
		ImGui::Text("Max Positive Deviation");
		ImGui::InputDouble("##MaxPositiveDeviation", &allowedPositiveDeviationFromReference_machineUnits, 0.0, 0.0, posDevString);
		if (allowedPositiveDeviationFromReference_machineUnits < 0.0) allowedPositiveDeviationFromReference_machineUnits = 0.0;
		ImGui::Text("Max Negative Deviation");
		ImGui::InputDouble("##MaxNegativeDeviation", &allowedNegativeDeviationFromReference_machineUnits, 0.0, 0.0, negDevString);
		if (allowedNegativeDeviationFromReference_machineUnits > 0.0) allowedNegativeDeviationFromReference_machineUnits = 0.0;
		ImGui::TextWrapped("No Limit Signal, the machine position is limited in reference to the feedback position."
			"\nSetting of the origin has to be done by manually moving the machine to the desired position reference and resetting the position feedback. (Not recommended for position feedback types other than absolute)");
		break;
	case PositionLimitType::Type::NO_LIMIT:
		ImGui::TextWrapped("No Position Limits."
			"\nMovement is unconstrained in every direction.");
		break;
	}
}











void SingleAxisMachine::devicesGui() {

	//======================== CONNECTED DEVICES ==========================

	glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 6, ImGui::GetTextLineHeight() * 1.5);

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Feedback: ");
	ImGui::PopFont();

	for (auto link : positionFeedbackDeviceLink->getLinks()) {

		std::shared_ptr<PositionFeedbackDevice> feedbackDevice = link->getInputData()->getPositionFeedbackDevice();

		ImGui::PushID(feedbackDevice->getName());

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("'%s' on device %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
		ImGui::PopFont();

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

		ImGui::PushStyleColor(ImGuiCol_Button, feedbackDevice->isOnline() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(feedbackDevice->isOnline() ? "Online" : "Offline", buttonSize);
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, feedbackDevice->isReady() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(feedbackDevice->isReady() ? "Ready" : "Not Ready", buttonSize);
		ImGui::PopStyleColor();

		ImGui::PopItemFlag();

		ImGui::PopID();

	}

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Reference: ");
	ImGui::PopFont();

	for (auto link : referenceDeviceLink->getLinks()) {

		std::shared_ptr<GpioDevice> gpioDevice = link->getInputData()->getGpioDevice();

		ImGui::PushID(gpioDevice->getName());

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("'%s' on device %s", gpioDevice->getName(), gpioDevice->parentDevice->getName());
		ImGui::PopFont();

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

		ImGui::PushStyleColor(ImGuiCol_Button, gpioDevice->isOnline() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(gpioDevice->isOnline() ? "Online" : "Offline", buttonSize);
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, gpioDevice->isReady() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(gpioDevice->isReady() ? "Ready" : "Not Ready", buttonSize);
		ImGui::PopStyleColor();


		ImGui::PopItemFlag();

		ImGui::PopID();

	}

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Actuators: ");
	ImGui::PopFont();

	for (auto link : actuatorDeviceLink->getLinks()) {

		std::shared_ptr<ActuatorDevice> actuatorDevice = link->getInputData()->getActuatorDevice();

		ImGui::PushID(actuatorDevice->getName());

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("'%s' on device %s", actuatorDevice->getName(), actuatorDevice->parentDevice->getName());
		ImGui::PopFont();

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

		ImGui::PushStyleColor(ImGuiCol_Button, actuatorDevice->isOnline() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(actuatorDevice->isOnline() ? "Online" : "Offline", buttonSize);
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, actuatorDevice->isReady() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(actuatorDevice->isReady() ? "Ready" : "Not Ready", buttonSize);
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, actuatorDevice->isEnabled() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
		ImGui::Button(actuatorDevice->isEnabled() ? "Enabled" : "Disabled", buttonSize);
		ImGui::PopStyleColor();


		ImGui::PopItemFlag();

		if (actuatorDevice->isEnabled()) { if (ImGui::Button("Disable", buttonSize)) actuatorDevice->disable(); }
		else if (ImGui::Button("Enable", buttonSize)) actuatorDevice->enable();

		ImGui::PopID();

	}

}



void SingleAxisMachine::metricsGui() {

	glm::vec2* positionBuffer;
	size_t positionPointCount = positionHistory.getBuffer(&positionBuffer);

	glm::vec2* actualPositionBuffer;
	size_t actualPositionPointCount = actualPositionHistory.getBuffer(&actualPositionBuffer);

	glm::vec2* positionErrorBuffer;
	size_t positionErrorPointCount = positionErrorHistory.getBuffer(&positionErrorBuffer);

	glm::vec2* velocityBuffer;
	size_t velocityPointCount = velocityHistory.getBuffer(&velocityBuffer);

	glm::vec2* accelerationBuffer;
	size_t accelerationPointCount = accelerationHistory.getBuffer(&accelerationBuffer);

	glm::vec2* loadBuffer;
	size_t loadPointCount = loadHistory.getBuffer(&loadBuffer);

	static int bufferSkip = 4;
	static double maxPositionFitOffset = 5.0;

	float zero = 0.0;
	float one = 1.0;
	float maxV = velocityLimit_machineUnitsPerSecond;
	float minV = -velocityLimit_machineUnitsPerSecond;
	float minA = -accelerationLimit_machineUnitsPerSecondSquared;
	float maxA = accelerationLimit_machineUnitsPerSecondSquared;

	if (positionPointCount > bufferSkip) {
		ImPlot::SetNextPlotLimitsX(positionBuffer[bufferSkip].x, positionBuffer[positionPointCount - bufferSkip].x, ImGuiCond_Always);
		ImPlot::FitNextPlotAxes(false, true, false, false);
	}
	ImPlot::SetNextPlotLimitsY(minV * 1.1, maxV * 1.1, ImGuiCond_Always, ImPlotYAxis_2);
	ImPlot::SetNextPlotLimitsY(minA * 1.1, maxA * 1.1, ImGuiCond_Always, ImPlotYAxis_3);


	ImPlotFlags plot1Flags = ImPlotFlags_AntiAliased | ImPlotFlags_NoChild | ImPlotFlags_YAxis2 | ImPlotFlags_YAxis3 | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus | ImPlotFlags_NoMousePos;

	if(ImPlot::BeginPlot("##Metrics", 0, 0, glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 20.0), plot1Flags)) {

		ImPlot::SetPlotYAxis(ImPlotYAxis_2);
		ImPlot::SetNextLineStyle(glm::vec4(0.3, 0.3, 0.3, 1.0), 2.0);
		ImPlot::PlotHLines("##zero", &zero, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("##MaxV", &maxV, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("##MinV", &minV, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 1.0, 1.0), 4.0);
		if (velocityPointCount > bufferSkip) ImPlot::PlotLine("Profile Velocity", &velocityBuffer[bufferSkip].x, &velocityBuffer[bufferSkip].y, velocityPointCount- bufferSkip, 0, sizeof(glm::vec2));
			
		ImPlot::SetPlotYAxis(ImPlotYAxis_3);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 0.3), 2.0);
		ImPlot::PlotHLines("##MaxA", &maxA, 1);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 0.3), 2.0);
		ImPlot::PlotHLines("##MinA", &minA, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.5, 0.5, 0.5, 1.0), 4.0);
		if (accelerationPointCount > bufferSkip) ImPlot::PlotLine("Profile Acceleration", &accelerationBuffer[bufferSkip].x, &accelerationBuffer[bufferSkip].y, accelerationPointCount- bufferSkip, 0, sizeof(glm::vec2));

		ImPlot::SetPlotYAxis(ImPlotYAxis_1);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 4.0);
		if (positionPointCount > bufferSkip) {
			ImPlot::PlotLine("Profile Position", &positionBuffer[bufferSkip].x, &positionBuffer[bufferSkip].y, positionPointCount - bufferSkip, 0, sizeof(glm::vec2));
			double pUp = positionBuffer[positionPointCount - 1].y + maxPositionFitOffset;
			double pDown = positionBuffer[positionPointCount - 1].y - maxPositionFitOffset;
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 0.0), 0.0);
			ImPlot::PlotHLines("##upperbound", &pUp, 1);
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 0.0), 0.0);
			ImPlot::PlotHLines("##lowerbound", &pDown, 1);
		}
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 0.0, 1.0), 2.0);
		if (actualPositionPointCount > bufferSkip) ImPlot::PlotLine("Actual Position", &actualPositionBuffer[bufferSkip].x, &actualPositionBuffer[bufferSkip].y, actualPositionPointCount - bufferSkip, 0, sizeof(glm::vec2));

		ImPlot::EndPlot();
	}


	ImPlotFlags plot2Flags = ImPlotFlags_AntiAliased | ImPlotFlags_NoChild | ImPlotFlags_YAxis2;

	if (positionErrorPointCount > bufferSkip) {
		ImPlot::SetNextPlotLimitsX(positionErrorBuffer[bufferSkip].x, positionErrorBuffer[positionErrorPointCount - bufferSkip].x, ImGuiCond_Always);
	}
	ImPlot::SetNextPlotLimitsY(-1.0, 1.0, ImGuiCond_Always, ImPlotYAxis_1);
	ImPlot::SetNextPlotLimitsY(-0.1, 1.1, ImGuiCond_Always, ImPlotYAxis_2);

	if (ImPlot::BeginPlot("##LoadAndError", 0, 0, glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 20.0), plot2Flags)) {

		ImPlot::SetPlotYAxis(ImPlotYAxis_1);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 0.0, 1.0), 2.0);
		if (positionErrorPointCount > bufferSkip) ImPlot::PlotLine("Position Error", &positionErrorBuffer[bufferSkip].x, &positionErrorBuffer[bufferSkip].y, positionErrorPointCount - bufferSkip, 0, sizeof(glm::vec2));
		ImPlot::SetPlotYAxis(ImPlotYAxis_2);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 1.0, 0.5), 2.0);
		ImPlot::PlotHLines("##zero", &zero, 1);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 1.0, 0.5), 2.0);
		ImPlot::PlotHLines("##one", &one, 1);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 1.0, 1.0), 2.0);
		if (loadPointCount > bufferSkip) ImPlot::PlotLine("Load", &loadBuffer[bufferSkip].x, &loadBuffer[bufferSkip].y, loadPointCount - bufferSkip, 0, sizeof(glm::vec2));

		ImPlot::EndPlot();
	}

}