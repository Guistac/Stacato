#include <pch.h>

#include "Motion/Machine/SingleAxisMachine.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Framework/Fonts.h"
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
	if (!readyToEnable) BEGIN_DISABLE_IMGUI_ELEMENT
		if (isEnabled()) {
			if (ImGui::Button("Disable Machine", buttonSize)) disable();
		}
		else {
			if (ImGui::Button("Enable Machine", buttonSize)) enable();
		}
	if (!readyToEnable) END_DISABLE_IMGUI_ELEMENT

		ImGui::Separator();

	bool disableManualControls = !b_enabled;
	if (disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT

		//------------------- MASTER MANUAL ACCELERATION ------------------------

		ImGui::Text("Acceleration for manual controls :");
	static char accelerationString[32];
	sprintf(accelerationString, u8"%.3f %s/s²", manualControlAcceleration_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
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

	float manualVelocityTarget = manualVelocityTarget_machineUnitsPerSecond;
	if (ImGui::SliderFloat("##Velocity", &manualVelocityTarget, -velocityLimit_machineUnitsPerSecond, velocityLimit_machineUnitsPerSecond, velocityTargetString));
	clamp(manualVelocityTarget_machineUnitsPerSecond, -velocityLimit_machineUnitsPerSecond, velocityLimit_machineUnitsPerSecond);
	if (ImGui::IsItemActive()) setVelocity(manualVelocityTarget);
	else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocity(manualVelocityTarget = 0.0);

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

	//actual position in range
	double minPosition = 0.0;
	double maxPosition = 0.0;
	double positionProgress = 0.0;
	static char positionString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		positionProgress = 1.0;
		sprintf(positionString, "Machine Disabled");
	}
	else {
		minPosition = getLowAxisPositionLimit();
		maxPosition = getHighAxisPositionLimit();
		positionProgress = getAxisPositionProgress();
		sprintf(positionString, "%.2f %s", actualPosition_machineUnits, getPositionUnit(machinePositionUnit)->shortForm);
		if (actualPosition_machineUnits < minPosition || actualPosition_machineUnits > maxPosition)
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	ImGui::Text("Current Position : (in range from %.2f%s to %.2f%s)", minPosition, getPositionUnitStringShort(machinePositionUnit), maxPosition, getPositionUnitStringShort(machinePositionUnit));
	ImGui::ProgressBar(positionProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), positionString);
	ImGui::PopStyleColor();
	

	//actual velocity
	float velocityProgress;
	static char velocityString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		velocityProgress = 1.0;
		sprintf(velocityString, "Machine Disabled");
	}
	else {
		velocityProgress = std::abs(actualVelocity_machineUnitsPerSecond) / velocityLimit_machineUnitsPerSecond;
		sprintf(velocityString, "%.2f %s/s", actualVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
		if (std::abs(actualVelocity_machineUnitsPerSecond) > std::abs(velocityLimit_machineUnitsPerSecond))
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}

	ImGui::Text("Current Velocity : (max %.2f%s/s)", velocityLimit_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), velocityString);
	ImGui::PopStyleColor();

	//target movement progress
	float targetProgress;
	double movementSecondsLeft = 0.0;
	static char movementProgressChar[8];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "Machine Disabled");
		targetProgress = 1.0;
	}
	else if (controlMode != ControlMode::POSITION_TARGET) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "No Target Movement");
		targetProgress = 1.0;
	}
	else if (MotionCurve::getMotionCurveProgress(currentProfilePointTime_seconds, targetCurveProfile) >= 1.0) {
		targetProgress = 1.0;
		sprintf(movementProgressChar, "Movement Finished");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	else {
		targetProgress = MotionCurve::getMotionCurveProgress(currentProfilePointTime_seconds, targetCurveProfile);
		movementSecondsLeft = targetCurveProfile.rampOutEndTime - currentProfilePointTime_seconds;
		if (movementSecondsLeft < 0.0) movementSecondsLeft = 0.0;
		sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
	}
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), movementProgressChar);
	ImGui::PopStyleColor();
	
	//encoder position in working range
	if ((needsPositionFeedbackDevice() && isPositionFeedbackDeviceConnected()) || (needsServoActuatorDevice() && isServoActuatorDeviceConnected())) {
		double range;
		double feedbackPosition_deviceUnits;
		double rangeMin_deviceUnits;
		double rangeMax_deviceUnits;
		PositionUnit::Unit devicePositionUnit;
		static char rangeString[64];
		bool deviceReady = false;

		if (needsPositionFeedbackDevice()) {
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getPositionFeedbackDevice();
			deviceReady = feedbackDevice->isReady();
			range = feedbackDevice->getPositionInRange();
			rangeMin_deviceUnits = feedbackDevice->getMinPosition();
			rangeMax_deviceUnits = feedbackDevice->getMaxPosition();
			devicePositionUnit = feedbackDevice->positionUnit;
			feedbackPosition_deviceUnits = feedbackDevice->getPosition();
		}
		else if (needsServoActuatorDevice()) {
			std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
			deviceReady = servo->isReady();
			range = servo->getPositionInRange();
			rangeMin_deviceUnits = servo->getMinPosition();
			rangeMax_deviceUnits = servo->getMaxPosition();
			devicePositionUnit = servo->positionUnit;
			feedbackPosition_deviceUnits = servo->getPosition();
		}

		if (!isEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
			range = 1.0;
			sprintf(rangeString, "Machine Disabled");
		}
		else if (range < 1.0 && range > 0.0) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
			sprintf(rangeString, "%.2f%s", feedbackPosition_deviceUnits, getPositionUnit(devicePositionUnit)->shortForm);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
			double distanceOutsideRange = range > 1.0 ?  feedbackPosition_deviceUnits - rangeMax_deviceUnits : feedbackPosition_deviceUnits - rangeMin_deviceUnits;
			sprintf(rangeString, "Encoder Outside Working Range by %.2f%s", distanceOutsideRange, getPositionUnit(devicePositionUnit)->shortForm);
			range = 1.0;
		}
		ImGui::Text("Feedback Position in Working Range : (%.2f%s to %.2f%s)", rangeMin_deviceUnits, getPositionUnit(devicePositionUnit)->shortForm, rangeMax_deviceUnits, getPositionUnit(devicePositionUnit)->shortForm);
		ImGui::ProgressBar(range, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), rangeString);
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

	//-------------------------- HOMING CONTROLS ---------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Origin and Limit Setting");
	ImGui::PopFont();

	static char positionScalingString[32];
	sprintf(positionScalingString, "%.3f %s", machineScalingPosition_machineUnits, getPositionUnit(machinePositionUnit)->shortForm);

	glm::vec2 homingButtonSize((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 1.5);

	switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			ImGui::TextWrapped("The Homing Sequence will move the axis in the negative direction until the negative limit signal is triggered."
			"\nThe Axis Origin will be set at the negative limit signal."
			"\nThe Positive Limit is set by manually moving the axis to the desired position and capturing it as the limit.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			ImGui::TextWrapped("The Homing Sequence will move the axis in both directions until each limit signal is triggered."
				"\nThe Axis Origin will be set at the negative limit signal."
				"\nThe Positive Limit will be set at the positive limit signal.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			ImGui::TextWrapped("The Homing Sequence will move the axis until the reference signal is triggered."
				"\nThe Axis Origin will be set at the reference signal."
				"\nThe Positive and Negative limits can be set by manually moving the axis to the desired limits and capturing the positions.");
			break;
		default:
			break;
	}

	switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:{
				bool homing = isHoming();
				if (homing) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Start Homing", homingButtonSize)) startHoming();
				if (homing) END_DISABLE_IMGUI_ELEMENT
				ImGui::SameLine();
				if (!homing) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Stop Homing", homingButtonSize)) cancelHoming();
				if (!homing) END_DISABLE_IMGUI_ELEMENT
				if (isHoming()) ImGui::Text("Homing Progress: %s", getHomingStep(homingStep)->displayName);
				else if (didHomingSucceed()) ImGui::Text("Homing Finished");
				else if (didHomingFail()) ImGui::Text("Homing Failed: %s", getHomingError(homingError)->displayName);
				else ImGui::Text("No Homing Ongoing");
			}break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			break;
	}

	if (isEnabled()) {
		ImGui::Text("Low Limit: %.3f %s", getLowAxisPositionLimit(), getPositionUnit(machinePositionUnit)->shortForm);
		ImGui::Text("High Limit: %.3f %s", getHighAxisPositionLimit(), getPositionUnit(machinePositionUnit)->shortForm);
	}

	switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			if (ImGui::Button("Capture Positive Limit", homingButtonSize)) setCurrentPositionAsPositiveLimit();
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			if (ImGui::Button("Capture Positive Limit", homingButtonSize)) setCurrentPositionAsNegativeLimit();
			if (ImGui::Button("Capture Negative Limit", homingButtonSize)) setCurrentPositionAsPositiveLimit();
			break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			if (ImGui::Button("Capture Origin", homingButtonSize)) setCurrentAxisPosition(0.0);
			break;
	}
	
	/*
	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Feedback Coupling Setting");
	ImGui::PopFont();

	ImGui::TextWrapped("After setting the origin");
	ImGui::SetNextItemWidth(homingButtonSize.x);
	ImGui::InputDouble("##posScal", &machineScalingPosition_machineUnits, 0.0, 0.0, positionScalingString);
	ImGui::SameLine();
	if (ImGui::Button("Set Position", ImGui::GetItemRectSize())) {}
	*/

	if (disableManualControls) END_DISABLE_IMGUI_ELEMENT
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
						positionReferenceSignal = getAngularPositionReferenceSignals().front().type;
						break;
					case PositionUnit::Type::LINEAR:
						machinePositionUnit = getLinearPositionUnits().front().unit;
						positionReferenceSignal = getLinearPositionReferenceSignals().front().type;
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

	ImGui::Text("Motion Control Type :");
	if (ImGui::BeginCombo("##MotionControlType", getPositionControlType(positionControl)->displayName)) {
		for (PositionControl& control : getPositionControlTypes()) {
			if (ImGui::Selectable(control.displayName, positionControl == control.type)) {
				setPositionControlType(control.type);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetNextWindowSize(glm::vec2(ImGui::GetTextLineHeight() * 20.0, 0));
				ImGui::BeginTooltip();
				switch (control.type) {
					case PositionControl::Type::SERVO:
						ImGui::TextWrapped("In this mode, closed loop control takes place in the servo drive itself."
							"\nA Position command is sent to the drive and the drive reports its current position."
							"\nCompatible only with Servo Actuators expecting Position Commands.");
						break;
					case PositionControl::Type::CLOSED_LOOP:
						ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
						ImGui::TextWrapped("Closed Loop Controlled Machines are not yet Supported.");
						ImGui::PopStyleColor();
						ImGui::TextWrapped("In this control mode, position feedback is used to to update a PID controller which regulates axis position by sending velocity commands."
							"\nCompatible only with Actuators expecting Velocity Commands.");
						break;
				}
				ImGui::EndTooltip();
			}
		}
		ImGui::EndCombo();
	}


	//---------------------- POSITION FEEDBACK ---------------------------

	if (needsPositionFeedbackDevice()) {

		ImGui::Separator();

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
				default: break;
			}

			if (feedbackAndActuatorConversionIdentical) BEGIN_DISABLE_IMGUI_ELEMENT
			ImGui::Text("%s %s per Machine %s :", feedbackDevice->getName(), getPositionUnit(feedbackDevice->positionUnit)->displayNamePlural, getPositionUnit(machinePositionUnit)->displayName);
			ImGui::InputDouble("##feedbackCoupling", &feedbackUnitsPerMachineUnits);
			if (feedbackAndActuatorConversionIdentical) END_DISABLE_IMGUI_ELEMENT

		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Feedback device connected.");
			ImGui::PopStyleColor();
		}
	}

	//---------------------- ACTUATOR -------------------------

	if (needsActuatorDevice()) {

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Actuator");
		ImGui::PopFont();

		if (isActuatorDeviceConnected()) {
			std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Device:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s on %s", actuator->getName(), actuator->parentDevice->getName());

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Position Unit:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s", getPositionUnit(actuator->positionUnit)->displayNamePlural);

			ImGui::Text("%s %s per Machine %s :", actuator->getName(), getPositionUnit(actuator->positionUnit)->displayNamePlural, getPositionUnit(machinePositionUnit)->displayName);
			ImGui::InputDouble("##actuatorCoupling", &actuatorUnitsPerMachineUnits);

			ImGui::Checkbox("##actFeedIdent", &feedbackAndActuatorConversionIdentical);
			ImGui::SameLine();
			ImGui::Text("Actuator and Feedback Coupling are identical");

			if (feedbackAndActuatorConversionIdentical) feedbackUnitsPerMachineUnits = actuatorUnitsPerMachineUnits;
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Actuator device connected.");
			ImGui::PopStyleColor();
		}
	}


	//------------------------ SERVO ACTUATOR -----------------------------

	if (needsServoActuatorDevice()) {

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Servo Actuator");
		ImGui::PopFont();

		if (isServoActuatorDeviceConnected()) {
		
			std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Device:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s on %s", servo->getName(), servo->parentDevice->getName());

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Position Unit:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s", getPositionUnit(servo->positionUnit)->displayNamePlural);

			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Feedback Type:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s", getPositionFeedbackType(servo->feedbackType)->displayName);

			switch (servo->feedbackType) {
			case PositionFeedback::Type::INCREMENTAL_FEEDBACK:
				ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
				ImGui::TextWrapped("With incremental position feedback, the homing routine needs to be executed on each system power cycle");
				ImGui::PopStyleColor();
				break;
			default: break;
			}

			ImGui::Text("%s %s per Machine %s :", servo->getName(), getPositionUnit(servo->positionUnit)->displayNamePlural, getPositionUnit(machinePositionUnit)->displayName);
			ImGui::InputDouble("##servoActuatorCoupling", &actuatorUnitsPerMachineUnits);
			feedbackUnitsPerMachineUnits = actuatorUnitsPerMachineUnits;

		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Servo Actuator device connected.");
			ImGui::PopStyleColor();
		}
		
	}

	//-------------------------- KINEMATIC LIMITS ----------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Kinematic Limits");
	ImGui::PopFont();

	if ((needsActuatorDevice() && isActuatorDeviceConnected()) || (needsServoActuatorDevice() && isServoActuatorDeviceConnected())) {
		double actuatorVelocityLimit_actuatorUnitsPerSecond;
		double actuatorAccelerationLimit_actuatorUnitsPerSecondSquared;
		double actuatorVelocityLimit_machineUnitsPerSecond;
		double actuatorAccelerationLimit_machineUnitsPerSecondSquared;
		PositionUnit::Unit actuatorUnit;
		if (needsActuatorDevice() && isActuatorDeviceConnected()) {
			std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
			actuatorVelocityLimit_actuatorUnitsPerSecond = actuator->getVelocityLimit();
			actuatorAccelerationLimit_actuatorUnitsPerSecondSquared = actuator->getAccelerationLimit();
			actuatorUnit = actuator->positionUnit;
		}
		else if (needsServoActuatorDevice() && isServoActuatorDeviceConnected()) {
			std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
			actuatorVelocityLimit_actuatorUnitsPerSecond = servo->getVelocityLimit();
			actuatorAccelerationLimit_actuatorUnitsPerSecondSquared = servo->getAccelerationLimit();
			actuatorUnit = servo->positionUnit;
		}
		actuatorVelocityLimit_machineUnitsPerSecond = actuatorVelocityLimit_actuatorUnitsPerSecond / actuatorUnitsPerMachineUnits;
		actuatorAccelerationLimit_machineUnitsPerSecondSquared = actuatorAccelerationLimit_actuatorUnitsPerSecondSquared / actuatorUnitsPerMachineUnits;
		ImGui::TextWrapped(u8"Max actuator velocity is %.1f %s/s and max acceleration is %.1f %s/s²",
			actuatorVelocityLimit_actuatorUnitsPerSecond,
			getPositionUnit(actuatorUnit)->shortForm,
			actuatorAccelerationLimit_actuatorUnitsPerSecondSquared,
			getPositionUnit(actuatorUnit)->shortForm);
		ImGui::TextWrapped(u8"Machine is limited to %.3f %s/s and %.3f %s/s²",
			actuatorVelocityLimit_machineUnitsPerSecond,
			getPositionUnit(machinePositionUnit)->shortForm,
			actuatorAccelerationLimit_machineUnitsPerSecondSquared,
			getPositionUnit(machinePositionUnit)->shortForm);
		clamp(velocityLimit_machineUnitsPerSecond, 0.0, actuatorVelocityLimit_machineUnitsPerSecond);
		clamp(accelerationLimit_machineUnitsPerSecondSquared, 0.0, actuatorAccelerationLimit_machineUnitsPerSecondSquared);
		clamp(defaultManualVelocity_machineUnitsPerSecond, 0.0, actuatorVelocityLimit_machineUnitsPerSecond);
		clamp(defaultManualAcceleration_machineUnitsPerSecondSquared, 0.0, actuatorAccelerationLimit_machineUnitsPerSecondSquared);	
	}

	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", velocityLimit_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));
	ImGui::InputDouble("##VelLimit", &velocityLimit_machineUnitsPerSecond, 0.0, 0.0, velLimitString);
	static char accLimitString[16];
	sprintf(accLimitString, u8"%.3f %s/s²", accelerationLimit_machineUnitsPerSecondSquared, getPositionUnitStringShort(machinePositionUnit));
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &accelerationLimit_machineUnitsPerSecondSquared, 0.0, 0.0, accLimitString);

	double halfWidgetWidth = (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;

	ImGui::Text("Default Manual Control Parameters", getPositionUnitStringPlural(machinePositionUnit));
	ImGui::SetNextItemWidth(halfWidgetWidth);
	static char manAccString[16];
	sprintf(manAccString, u8"%.3f %s/s²", defaultManualAcceleration_machineUnitsPerSecondSquared, getPositionUnitStringShort(machinePositionUnit));
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
	ImGui::Text("Position Reference and Limits");
	ImGui::PopFont();

	static auto showPositionReferenceDescription = [](PositionReferenceSignal::Type type) {
		switch (type) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			ImGui::TextWrapped("Single Limit Signal at the negative end of the machine travel."
				"\nHoming will move the machine in the negative direction"
				"\Axis Origin is set at the low limit.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			ImGui::TextWrapped("Two Limit Signals at each end of the axis travel."
				"\nHoming will first move the axis in the specified direction, then in the other direction"
				"\Axis Origin is set at the low limit.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			ImGui::TextWrapped("Single Reference Signal inside the axis travel range."
				"\nHoming will find the signal using the specified direction."
				"\nAxis Origin is set at the reference signal.");
			break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			ImGui::TextWrapped("No reference Signal, the machine is positionned using only feedback data."
				"\Axis origin is set by manually moving the machine to the desired position and resetting the position feedback.");
			break;
		}
	};

	if (ImGui::BeginCombo("##PositionReference", getPositionReferenceSignal(positionReferenceSignal)->displayName)) {
		switch (machinePositionUnitType) {
		case PositionUnit::Type::LINEAR:
			for (PositionReferenceSignal& reference : getLinearPositionReferenceSignals()) {
				bool selected = positionReferenceSignal == reference.type;
				if (ImGui::Selectable(reference.displayName, selected)) {
					setPositionReferenceSignalType(reference.type);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetNextWindowSize(glm::vec2(ImGui::GetTextLineHeight() * 20.0, 0));
					ImGui::BeginTooltip();
					showPositionReferenceDescription(reference.type);
					ImGui::EndTooltip();
				}
			}
			break;
		case PositionUnit::Type::ANGULAR:
			for (PositionReferenceSignal& reference : getAngularPositionReferenceSignals()) {
				bool selected = positionReferenceSignal == reference.type;
				if (ImGui::Selectable(reference.displayName, selected)) {
					setPositionReferenceSignalType(reference.type);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetNextWindowSize(glm::vec2(ImGui::GetTextLineHeight() * 20.0, 0));
					ImGui::BeginTooltip();
					showPositionReferenceDescription(reference.type);
					ImGui::EndTooltip();
				}
			}
			break;
		}
		ImGui::EndCombo();
	}

	bool showPositionReferenceSettings = true;
	if (needsReferenceDevice()) {
		if (isReferenceDeviceConnected()) {
			std::shared_ptr<GpioDevice> gpioDevice = getReferenceDevice();
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Reference Device:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s on %s", gpioDevice->getName(), gpioDevice->parentDevice->getName());
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Reference Device connected.");
			ImGui::PopStyleColor();
			showPositionReferenceSettings = false;
		}
	}

	if (showPositionReferenceSettings) {

		static char negDevString[16];
		sprintf(negDevString, "%.3f %s", maxNegativeDeviation_machineUnits, getPositionUnitStringShort(machinePositionUnit));
		static char posDevString[16];
		sprintf(posDevString, "%.3f %s", maxPositiveDeviation_machineUnits, getPositionUnitStringShort(machinePositionUnit));
		static char homVelString[16];
		sprintf(homVelString, "%.3f %s/s", homingVelocity_machineUnitsPerSecond, getPositionUnitStringShort(machinePositionUnit));

		switch (positionReferenceSignal) {
			case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
				ImGui::Text("Max Distance From Origin");
				ImGui::InputDouble("##MaxDeviation", &maxPositiveDeviation_machineUnits, 0.0, 0.0, posDevString);
				if (maxPositiveDeviation_machineUnits < 0.0) maxPositiveDeviation_machineUnits = 0.0;
				ImGui::Text("Homing Velocity", getPositionUnit(machinePositionUnit)->displayNamePlural);
				ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
				if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
				break;
			case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
				ImGui::Text("Homing Direction");
				if (ImGui::BeginCombo("##HomingDirection", getHomingDirection(homingDirection)->displayName)) {
					for (HomingDirection& direction : getHomingDirections()) {
						bool selected = homingDirection == direction.type;
						if (ImGui::Selectable(direction.displayName, selected)) homingDirection = direction.type;
					}
					ImGui::EndCombo();
				}
				ImGui::Text("Homing Velocity", getPositionUnit(machinePositionUnit)->displayNamePlural);
				ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
				if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
				break;
			case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
				ImGui::Text("Homing Direction");
				if (ImGui::BeginCombo("##HomingDirection", getHomingDirection(homingDirection)->displayName)) {
					for (HomingDirection& direction : getHomingDirections()) {
						bool selected = homingDirection == direction.type;
						if (ImGui::Selectable(direction.displayName, selected)) homingDirection = direction.type;
					}
					ImGui::EndCombo();
				}
				ImGui::Text("Max Positive Distance From Origin");
				ImGui::InputDouble("##MaxPositiveDeviation", &maxPositiveDeviation_machineUnits, 0.0, 0.0, posDevString);
				if (maxPositiveDeviation_machineUnits < 0.0) maxPositiveDeviation_machineUnits = 0.0;
				ImGui::Text("Max Negative Distance From Origin");
				ImGui::InputDouble("##MaxNegativeDeviation", &maxNegativeDeviation_machineUnits, 0.0, 0.0, negDevString);
				if (maxNegativeDeviation_machineUnits > 0.0) maxNegativeDeviation_machineUnits = 0.0;
				ImGui::Text("Homing Velocity", getPositionUnit(machinePositionUnit)->displayNamePlural);
				ImGui::InputDouble("##HomingVelocity", &homingVelocity_machineUnitsPerSecond, 0.0, 0.0, homVelString);
				if (homingVelocity_machineUnitsPerSecond < 0) homingVelocity_machineUnitsPerSecond = abs(homingVelocity_machineUnitsPerSecond);
				break;
			case PositionReferenceSignal::Type::NO_SIGNAL:
				ImGui::Text("Max Positive Distance From Origin");
				ImGui::InputDouble("##MaxPositiveDeviation", &maxPositiveDeviation_machineUnits, 0.0, 0.0, posDevString);
				if (maxPositiveDeviation_machineUnits < 0.0) maxPositiveDeviation_machineUnits = 0.0;
				ImGui::Text("Max Negative Distance From Origin");
				ImGui::InputDouble("##MaxNegativeDeviation", &maxNegativeDeviation_machineUnits, 0.0, 0.0, negDevString);
				if (maxNegativeDeviation_machineUnits > 0.0) maxNegativeDeviation_machineUnits = 0.0;
				break;
		}
	}
}





void SingleAxisMachine::devicesGui() {

	//======================== CONNECTED DEVICES ==========================

	glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 6, ImGui::GetTextLineHeight() * 1.5);

	if (needsPositionFeedbackDevice()) {
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback: ");
		ImGui::PopFont();
		if (isPositionFeedbackDeviceConnected()) {
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getPositionFeedbackDevice();
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
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Position Feedback Device connected.");
			ImGui::PopStyleColor();
		}
	}

	if (needsActuatorDevice()) {
		ImGui::Separator();
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Actuator:");
		ImGui::PopFont();
		if (isActuatorDeviceConnected()) {
			std::shared_ptr<ActuatorDevice> actuatorDevice = getActuatorDevice();
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
		}
	}
	else if (needsServoActuatorDevice()) {
		ImGui::Separator();
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Servo Actuator:");
		ImGui::PopFont();
		if (isServoActuatorDeviceConnected()) {
			std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("'%s' on device %s", servo->getName(), servo->parentDevice->getName());
			ImGui::PopFont();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_Button, servo->isOnline() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
			ImGui::Button(servo->isOnline() ? "Online" : "Offline", buttonSize);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, servo->isReady() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
			ImGui::Button(servo->isReady() ? "Ready" : "Not Ready", buttonSize);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, servo->isEnabled() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
			ImGui::Button(servo->isEnabled() ? "Enabled" : "Disabled", buttonSize);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
			if (servo->isEnabled()) { if (ImGui::Button("Disable", buttonSize)) servo->disable(); }
			else if (ImGui::Button("Enable", buttonSize)) servo->enable();
		}
	}

	if (needsReferenceDevice()) {
		ImGui::Separator();
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Reference: ");
		ImGui::PopFont();
		if (isReferenceDeviceConnected()) {
			std::shared_ptr<GpioDevice> gpioDevice = getReferenceDevice();
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
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Reference Device connected.");
			ImGui::PopStyleColor();
		}
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

	if (ImPlot::BeginPlot("##Metrics", 0, 0, glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 20.0), plot1Flags)) {

		ImPlot::SetPlotYAxis(ImPlotYAxis_2);
		ImPlot::SetNextLineStyle(glm::vec4(0.3, 0.3, 0.3, 1.0), 2.0);
		ImPlot::PlotHLines("##zero", &zero, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("##MaxV", &maxV, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
		ImPlot::PlotHLines("##MinV", &minV, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 1.0, 1.0), 4.0);
		if (velocityPointCount > bufferSkip) ImPlot::PlotLine("Profile Velocity", &velocityBuffer[bufferSkip].x, &velocityBuffer[bufferSkip].y, velocityPointCount - bufferSkip, 0, sizeof(glm::vec2));

		ImPlot::SetPlotYAxis(ImPlotYAxis_3);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 0.3), 2.0);
		ImPlot::PlotHLines("##MaxA", &maxA, 1);
		ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 0.3), 2.0);
		ImPlot::PlotHLines("##MinA", &minA, 1);
		ImPlot::SetNextLineStyle(glm::vec4(0.5, 0.5, 0.5, 1.0), 4.0);
		if (accelerationPointCount > bufferSkip) ImPlot::PlotLine("Profile Acceleration", &accelerationBuffer[bufferSkip].x, &accelerationBuffer[bufferSkip].y, accelerationPointCount - bufferSkip, 0, sizeof(glm::vec2));

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