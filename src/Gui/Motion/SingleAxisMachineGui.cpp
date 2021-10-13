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

	bool isMachineReady = isReady();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Machine Enabled", buttonSize);
	}
	else if (isMachineReady) {
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
	if(!isMachineReady) BEGIN_DISABLE_IMGUI_ELEMENT
	if (isEnabled()) {
		if (ImGui::Button("Disable Machine", buttonSize)) disable();
	}
	else {
		if (ImGui::Button("Enable Machine", buttonSize)) enable();
	}
	if (!isMachineReady) END_DISABLE_IMGUI_ELEMENT

	ImGui::Separator();

	bool disableManualControls = !b_enabled;
	if(disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT

	//------------------- MASTER MANUAL ACCELERATION ------------------------

	ImGui::Text("Acceleration for manual controls :");
	ImGui::InputDouble("##TargetAcceleration", &manualControlAcceleration_degreesPerSecond, 0.0, 0.0, "%.3f deg/s2");
	clamp(manualControlAcceleration_degreesPerSecond, 0.0, accelerationLimit_degreesPerSecondSquared);
	ImGui::Separator();

	//------------------- VELOCITY CONTROLS ------------------------

	float widgetWidth = ImGui::GetContentRegionAvail().x;

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	ImGui::SetNextItemWidth(widgetWidth);
	if (ImGui::SliderFloat("##Velocity", &manualVelocityTarget_degreesPerSecond, -velocityLimit_degreesPerSecond, velocityLimit_degreesPerSecond, "%.3f deg/s")) {
		clamp(manualVelocityTarget_degreesPerSecond, -velocityLimit_degreesPerSecond, velocityLimit_degreesPerSecond);
		setVelocity(manualVelocityTarget_degreesPerSecond);
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
	ImGui::InputDouble("##TargetPosition", &targetPosition, 0.0, 0.0, "%.3f deg");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetVelocity", &targetVelocity, 0.0, 0.0, "%.3f deg/s");
	clamp(targetVelocity, 0.0, velocityLimit_degreesPerSecond);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetTime", &targetTime, 0.0, 0.0, "%.3f s");
	if (ImGui::Button("Fast Move", tripleButtonSize)) {
		moveToPositionWithVelocity(targetPosition, velocityLimit_degreesPerSecond, manualControlAcceleration_degreesPerSecond);
	}
	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize)) {
		moveToPositionWithVelocity(targetPosition, targetVelocity, manualControlAcceleration_degreesPerSecond);
	}
	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize)) {
		moveToPositionInTime(targetPosition, targetTime, manualControlAcceleration_degreesPerSecond);
	}

	if (ImGui::Button("Stop##Target", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) {
		setVelocity(0.0);
	}

	ImGui::Separator();

	//-------------------------------- FEEDBACK --------------------------------

		
	if (std::abs(profileVelocity_degreesPerSecond) == std::abs(velocityLimit_degreesPerSecond)) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
	else if (std::abs(profileVelocity_degreesPerSecond) > std::abs(velocityLimit_degreesPerSecond)) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
	else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	char velocityString[16];
	sprintf(velocityString, "%.2f u/s", profileVelocity_degreesPerSecond);
	float velocityProgress = std::abs(profileVelocity_degreesPerSecond) / velocityLimit_degreesPerSecond;
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
	char movementProgressChar[8];
	sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), movementProgressChar);
	ImGui::PopStyleColor();


	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice = nullptr;
	double range;
	char rangeString[64];
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

	switch (positionReferenceType) {
	case PositionReference::Type::LOW_LIMIT:
	case PositionReference::Type::HIGH_LIMIT:
	case PositionReference::Type::LOW_AND_HIGH_LIMIT:
	case PositionReference::Type::POSITION_REFERENCE:
		if (ImGui::Button("Start Homing", tripleButtonSize)) {}
		ImGui::SameLine();
		if (ImGui::Button("Cancel Homing", tripleButtonSize)) {}
		break;
	case PositionReference::Type::NO_LIMIT:
		if (ImGui::Button("Reset Position Feedback")) {}
		break;
	}

	if(disableManualControls) END_DISABLE_IMGUI_ELEMENT

}








void SingleAxisMachine::settingsGui() {

	//=================== AXIS SETTINGS ====================

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();

	//------------------ AXIS TYPE -------------------------

	ImGui::Text("Position Unit Type");
	if (ImGui::BeginCombo("##AxisUnitType", getPositionUnitType(axisPositionUnitType)->displayName)) {
		for (PositionUnitType& unitType : getPositionUnitTypes()) {
			if (ImGui::Selectable(unitType.displayName, axisPositionUnitType == unitType.type)) {
				axisPositionUnitType = unitType.type;
				//if the machine type is changed but the machine unit is of the wrong type
				//change the machine unit to the first correct type automatically
				if (getPositionUnit(axisPositionUnit)->type != unitType.type) {
					if (unitType.type == PositionUnit::Type::ANGULAR) {
						axisPositionUnit = getAngularPositionUnits().front().unit;
					}
					else if (unitType.type == PositionUnit::Type::LINEAR) {
						axisPositionUnit = getLinearPositionUnits().front().unit;
					}
				}
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Position Unit");
	if (ImGui::BeginCombo("##AxisUnit", getPositionUnit(axisPositionUnit)->displayName)) {
		if (axisPositionUnitType == PositionUnit::Type::LINEAR) {
			for (PositionUnit& unit : getLinearPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, axisPositionUnit == unit.unit)) axisPositionUnit = unit.unit;
			}
		}
		else if (axisPositionUnitType == PositionUnit::Type::ANGULAR) {
			for (PositionUnit& unit : getAngularPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, axisPositionUnit == unit.unit)) axisPositionUnit = unit.unit;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	//---------------------- POSITION FEEDBACK ---------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Feedback");
	ImGui::PopFont();

	ImGui::Text("Feedback Type");
	if (ImGui::BeginCombo("##FeedbackType", getPositionFeedbackType(positionFeedbackType)->displayName)) {
		for (PositionFeedback& feedback : getPositionFeedbackTypes()) {
			if (ImGui::Selectable(feedback.displayName, positionFeedbackType == feedback.type)) positionFeedbackType = feedback.type;
		}
		ImGui::EndCombo();
	}

	if (positionFeedbackType == PositionFeedback::Type::NO_FEEDBACK) {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("Axes without position feedback are not yet supported. Without position feedback, we would need calibration on each software start and then do open loop control");
		ImGui::PopStyleColor();
	}
	else if (positionFeedbackType == PositionFeedback::Type::INCREMENTAL_FEEDBACK) {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("Axes with incremental position feedback are not yet supported. With incremental position feedback, we would be forced to perform a homing routine on each system power cycle");
		ImGui::PopStyleColor();
	}
	else {

		ImGui::Text("Position Unit Type");
		if (ImGui::BeginCombo("##FeedbackUnitType", getPositionUnitType(feedbackPositionUnitType)->displayName)) {
			for (PositionUnitType& unitType : getPositionUnitTypes()) {
				if (ImGui::Selectable(unitType.displayName, feedbackPositionUnitType == unitType.type)) {
					feedbackPositionUnitType = unitType.type;
					//if the machine type is changed but the machine unit is of the wrong type
					//change the machine unit to the first correct type automatically
					if (getPositionUnit(feedbackPositionUnit)->type != unitType.type) {
						if (unitType.type == PositionUnit::Type::ANGULAR) {
							feedbackPositionUnit = getAngularPositionUnits().front().unit;
						}
						else if (unitType.type == PositionUnit::Type::LINEAR) {
							feedbackPositionUnit = getLinearPositionUnits().front().unit;
						}
					}
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Position Unit");
		if (ImGui::BeginCombo("##FeedbackUnit", getPositionUnit(feedbackPositionUnit)->displayName)) {
			if (feedbackPositionUnitType == PositionUnit::Type::LINEAR) {
				for (PositionUnit& unit : getLinearPositionUnits()) {
					if (ImGui::Selectable(unit.displayName, feedbackPositionUnit == unit.unit)) feedbackPositionUnit = unit.unit;
				}
			}
			else if (feedbackPositionUnitType == PositionUnit::Type::ANGULAR) {
				for (PositionUnit& unit : getAngularPositionUnits()) {
					if (ImGui::Selectable(unit.displayName, feedbackPositionUnit == unit.unit)) feedbackPositionUnit = unit.unit;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Feedback %s per Machine %s", getPositionUnit(feedbackPositionUnit)->displayNamePlural, getPositionUnit(axisPositionUnit)->displayName);
		ImGui::InputDouble("##feedbackCoupling", &feedbackUnitsPerAxisUnits);
	}

	ImGui::Separator();

	//---------------------- ACTUATOR AND COMMAND -------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Actuator");
	ImGui::PopFont();

	ImGui::Text("Actuator Command Type");
	if (ImGui::BeginCombo("##CommandType", getCommandType(actuatorCommandType)->displayName)) {
		for (CommandType& command : getCommandTypes()) {
			if (ImGui::Selectable(command.displayName, actuatorCommandType == command.type)) actuatorCommandType = command.type;
		}
		ImGui::EndCombo();
	}

	if (actuatorCommandType == CommandType::Type::VELOCITY_COMMAND) {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("Actuators with velocity command are not yet supported. Actuators with velocity command would need a PID controller with position feedback to regulate the machine position");
		ImGui::PopStyleColor();
	}
	else {

		ImGui::Text("Position Unit Type");
		if (ImGui::BeginCombo("##ActuatorUnitType", getPositionUnitType(actuatorPositionUnitType)->displayName)) {
			for (PositionUnitType& unitType : getPositionUnitTypes()) {
				if (ImGui::Selectable(unitType.displayName, actuatorPositionUnitType == unitType.type)) {
					actuatorPositionUnitType = unitType.type;
					//if the machine type is changed but the machine unit is of the wrong type
					//change the machine unit to the first correct type automatically
					if (getPositionUnit(actuatorPositionUnit)->type != unitType.type) {
						if (unitType.type == PositionUnit::Type::ANGULAR) {
							actuatorPositionUnit = getAngularPositionUnits().front().unit;
						}
						else if (unitType.type == PositionUnit::Type::LINEAR) {
							actuatorPositionUnit = getLinearPositionUnits().front().unit;
						}
					}
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Position Unit");
		if (ImGui::BeginCombo("##ActuatorUnit", getPositionUnit(actuatorPositionUnit)->displayName)) {
			if (actuatorPositionUnitType == PositionUnit::Type::LINEAR) {
				for (PositionUnit& unit : getLinearPositionUnits()) {
					if (ImGui::Selectable(unit.displayName, actuatorPositionUnit == unit.unit)) actuatorPositionUnit = unit.unit;
				}
			}
			else if (actuatorPositionUnitType == PositionUnit::Type::ANGULAR) {
				for (PositionUnit& unit : getAngularPositionUnits()) {
					if (ImGui::Selectable(unit.displayName, actuatorPositionUnit == unit.unit)) actuatorPositionUnit = unit.unit;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Actuator %s per Machine %s", getPositionUnit(actuatorPositionUnit)->displayNamePlural, getPositionUnit(axisPositionUnit)->displayName);
		ImGui::InputDouble("##actuatorCoupling", &actuatorUnitsPerAxisUnits);
	}

	ImGui::Separator();

	//-------------------------- KINEMATIC LIMITS ----------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Kinematic Limits");
	ImGui::PopFont();

	ImGui::Text("Velocity Limit (%s per second)", getMachinePositionUnitStringPlural());
	ImGui::InputDouble("##VelLimit", &velocityLimit_degreesPerSecond, 0.0, 0.0, "%.3f u/s");
	ImGui::Text("Acceleration Limit (%s per second squared)", getMachinePositionUnitStringPlural());
	ImGui::InputDouble("##AccLimit", &accelerationLimit_degreesPerSecondSquared, 0.0, 0.0, "%.3f u/s2");

	ImGui::Text("Default Manual Acceleration (%s per second squared)", getMachinePositionUnitStringPlural());
	ImGui::InputDouble("##defmanAcc", &defaultManualAcceleration_degreesPerSecondSquared, 0.0, 0.0, "%.3f u/s");
	clamp(defaultManualAcceleration_degreesPerSecondSquared, 0.0, accelerationLimit_degreesPerSecondSquared);
	ImGui::Text("Default Manual Velocity (%s per second)", getMachinePositionUnitStringPlural());
	ImGui::InputDouble("##defmanvel", &defaultManualVelocity_degreesPerSecond, 0.0, 0.0, "%.3f u/s");
	clamp(defaultManualVelocity_degreesPerSecond, 0.0, velocityLimit_degreesPerSecond);

	ImGui::Separator();

	//----------------- POSITION REFERENCES AND HOMING -----------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position References");
	ImGui::PopFont();

	ImGui::Text("Limit Type");
	if (ImGui::BeginCombo("##PositionReference", getPositionReferenceType(positionReferenceType)->displayName)) {
		for (PositionReference& reference : getPositionReferenceTypes()) {
			bool selected = positionReferenceType == reference.type;
			if (ImGui::Selectable(reference.displayName, selected)) positionReferenceType = reference.type;
		}
		ImGui::EndCombo();
	}

	switch (positionReferenceType) {
	case PositionReference::Type::LOW_LIMIT:
		ImGui::Text("Max Deviation From Low Limit");
		ImGui::InputDouble("##MaxDeviation", &allowedPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f deg");
		if (allowedPositiveDeviationFromReference_degrees < 0.0) allowedPositiveDeviationFromReference_degrees = 0.0;
		ImGui::Text("Homing Velocity");
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
		if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
		ImGui::TextWrapped("Single Limit Signal at the negative end of the machine travel. Homing will move the machine in the negative direction");
		break;
	case PositionReference::Type::HIGH_LIMIT:
		ImGui::Text("Max Deviation From High Limit");
		ImGui::InputDouble("##MaxDeviation", &allowedNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f deg");
		if (allowedNegativeDeviationFromReference_degrees > 0.0) allowedNegativeDeviationFromReference_degrees = 0.0;
		ImGui::Text("Homing Velocity");
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
		if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
		ImGui::TextWrapped("Single Limit Signal at the positive end of the machine travel. Homing will move the machine in the position direction");
		break;
	case PositionReference::Type::LOW_AND_HIGH_LIMIT:
		ImGui::Text("Homing Direction");
		if (ImGui::BeginCombo("##HomingDirection", getHomingDirectionType(homingDirectionType)->displayName)) {
			for (HomingDirection& direction : getHomingDirectionTypes()) {
				bool selected = homingDirectionType == direction.type;
				if (ImGui::Selectable(direction.displayName, selected)) homingDirectionType = direction.type;
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Homing Velocity");
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
		if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
		ImGui::TextWrapped("Two Limit Signals at each end of the machine travel. Homing will first move the machine in the specified direction, then the other direction");
		break;
	case PositionReference::Type::POSITION_REFERENCE:
		ImGui::Text("Homing Direction");
		if (ImGui::BeginCombo("##HomingDirection", getHomingDirectionType(homingDirectionType)->displayName)) {
			for (HomingDirection& direction : getHomingDirectionTypes()) {
				bool selected = homingDirectionType == direction.type;
				if (ImGui::Selectable(direction.displayName, selected)) homingDirectionType = direction.type;
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Max Positive Deviation");
		ImGui::InputDouble("##MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
		if (allowedPositiveDeviationFromReference_degrees < 0.0) allowedPositiveDeviationFromReference_degrees = 0.0;
		ImGui::Text("Max Negative Deviation");
		ImGui::InputDouble("##MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
		if (allowedNegativeDeviationFromReference_degrees > 0.0) allowedNegativeDeviationFromReference_degrees = 0.0;
		ImGui::Text("Homing Velocity");
		ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
		if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
		ImGui::TextWrapped("Single Limit Signal inside the machine travel range. Homing will find the position reference using the specified direction. The machine will not go over the max deviations from the position reference. (Not recommended for machine with physical limits)");
		break;
	case PositionReference::Type::NO_LIMIT:
		ImGui::Text("Max Positive Deviation");
		ImGui::InputDouble("##MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
		if (allowedPositiveDeviationFromReference_degrees < 0.0) allowedPositiveDeviationFromReference_degrees = 0.0;
		ImGui::Text("Max Negative Deviation");
		ImGui::InputDouble("##MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
		if (allowedNegativeDeviationFromReference_degrees > 0.0) allowedNegativeDeviationFromReference_degrees = 0.0;
		ImGui::TextWrapped("No Limit Signal. Setting of the origin has to be done by manually moving the machine to the desired position reference and resetting the position feedback. (Not recommended for position feedback types other than absolute)");
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

	for (auto link : referenceDeviceLinks->getLinks()) {

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

	for (auto link : actuatorDeviceLinks->getLinks()) {

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
	float maxV = velocityLimit_degreesPerSecond;
	float minV = -velocityLimit_degreesPerSecond;
	float minA = -accelerationLimit_degreesPerSecondSquared;
	float maxA = accelerationLimit_degreesPerSecondSquared;

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