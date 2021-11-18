#include <pch.h>

#include "Motion/Axis/PositionControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "NodeGraph/Device.h"
#include "Gui/Utilities/HelpMarker.h"

#include "Motion/SubDevice.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/Curve/Curve.h"

void PositionControlledAxis::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Control")) {

		stateControlGui();

		glm::vec2 controlChildSize = ImGui::GetContentRegionAvail();
		controlChildSize.y -= getFeedbackGuiHeight();

		ImGui::BeginChild("ControlsChild", controlChildSize);
		if (ImGui::BeginTabBar("ControlsTab")) {
			if (ImGui::BeginTabItem("Manual Controls")) {
				controlsGui();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Homing & Setup")) {
				setupGui();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
		ImGui::Separator();

		feedbackGui();

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		if (ImGui::BeginChild("Settings")) {
			settingsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		if (ImGui::BeginChild("Devices")) {
			devicesGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		if (ImGui::BeginChild("Metrics")) {
			metricsGui();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
}


void PositionControlledAxis::stateControlGui() {

	glm::vec2 buttonSize;
	int buttonCount = 2;
	buttonSize.x = (ImGui::GetContentRegionAvail().x - (buttonCount - 1) * ImGui::GetStyle().ItemSpacing.x) / buttonCount;
	buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Axis Enabled", buttonSize);
	}
	else if (isReady()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Axis Ready", buttonSize);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Axis Not Ready", buttonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();

	bool disableEnableButton = !isReady();// || !b_manualControlsEnabled;

	ImGui::SameLine();
	if (disableEnableButton) BEGIN_DISABLE_IMGUI_ELEMENT
		if (isEnabled()) {
			if (ImGui::Button("Disable Axis", buttonSize)) disable();
		}
		else {
			if (ImGui::Button("Enable Axis", buttonSize)) enable();
		}
	if (disableEnableButton) END_DISABLE_IMGUI_ELEMENT

}


void PositionControlledAxis::controlsGui() {

	//====================== AXIS MANUAL CONTROLS ==============================

	float widgetWidth = ImGui::GetContentRegionAvail().x;
	float tripleWidgetWidth = (widgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

	bool axisDisabled = !b_enabled;
	if (axisDisabled) BEGIN_DISABLE_IMGUI_ELEMENT

	//if (b_manualControlsEnabled) {

		//------------------- MASTER MANUAL ACCELERATION ------------------------

		ImGui::Text("Acceleration for manual controls :");
		static char accelerationString[32];
		sprintf(accelerationString, "%.3f %s/s\xc2\xb2", manualControlAcceleration_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));
		ImGui::InputDouble("##TargetAcceleration", &manualControlAcceleration_axisUnitsPerSecond, 0.0, 0.0, accelerationString);
		clampValue(manualControlAcceleration_axisUnitsPerSecond, 0.0, accelerationLimit_axisUnitsPerSecondSquared);
		ImGui::Separator();

		//------------------- VELOCITY CONTROLS ------------------------

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Velocity Control");
		ImGui::PopFont();

		ImGui::SetNextItemWidth(widgetWidth);
		static char velocityTargetString[32];
		sprintf(velocityTargetString, "%.3f %s/s", manualVelocityTarget_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));

		float manualVelocityTarget = manualVelocityTarget_axisUnitsPerSecond;
		if (ImGui::SliderFloat("##Velocity", &manualVelocityTarget, -velocityLimit_axisUnitsPerSecond, velocityLimit_axisUnitsPerSecond, velocityTargetString));
		clampValue(manualVelocityTarget_axisUnitsPerSecond, -velocityLimit_axisUnitsPerSecond, velocityLimit_axisUnitsPerSecond);
		if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTarget);
		else if (ImGui::IsItemDeactivatedAfterEdit()) setVelocityTarget(manualVelocityTarget = 0.0);

		ImGui::Separator();

		//------------------------- POSITION CONTROLS --------------------------

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Position Control");
		ImGui::PopFont();

		ImGui::SetNextItemWidth(tripleWidgetWidth);
		static char targetPositionString[32];
		sprintf(targetPositionString, "%.3f %s", targetPosition_axisUnits, getPositionUnitStringShort(positionUnit));
		ImGui::InputDouble("##TargetPosition", &targetPosition_axisUnits, 0.0, 0.0, targetPositionString);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tripleWidgetWidth);
		static char targetVelocityString[32];
		sprintf(targetVelocityString, "%.3f %s/s", targetVelocity_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));
		ImGui::InputDouble("##TargetVelocity", &targetVelocity_axisUnitsPerSecond, 0.0, 0.0, targetVelocityString);
		clampValue(targetVelocity_axisUnitsPerSecond, 0.0, velocityLimit_axisUnitsPerSecond);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tripleWidgetWidth);
		ImGui::InputDouble("##TargetTime", &targetTime_seconds, 0.0, 0.0, "%.3f s");
		if (ImGui::Button("Fast Move", tripleButtonSize)) {
			moveToPositionWithVelocity(targetPosition_axisUnits, velocityLimit_axisUnitsPerSecond, manualControlAcceleration_axisUnitsPerSecond);
		}
		ImGui::SameLine();
		if (ImGui::Button("Velocity Move", tripleButtonSize)) {
			moveToPositionWithVelocity(targetPosition_axisUnits, targetVelocity_axisUnitsPerSecond, manualControlAcceleration_axisUnitsPerSecond);
		}
		ImGui::SameLine();
		if (ImGui::Button("Timed Move", tripleButtonSize)) {
			moveToPositionInTime(targetPosition_axisUnits, targetTime_seconds, manualControlAcceleration_axisUnitsPerSecond);
		}

		float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;

		if (ImGui::Button("Stop", glm::vec2(doubleWidgetWidth, ImGui::GetTextLineHeight() * 2))) setVelocityTarget(0.0);
		ImGui::SameLine();
		if (ImGui::Button("Fast Stop", glm::vec2(doubleWidgetWidth, ImGui::GetTextLineHeight() * 2))) fastStop();
		

		ImGui::Separator();

	//}

	if (axisDisabled) END_DISABLE_IMGUI_ELEMENT
}








void PositionControlledAxis::feedbackGui() {

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Feedback");
	ImGui::PopFont();

	float widgetWidth = ImGui::GetContentRegionAvail().x;

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

		/*
		minPosition = getLowPositionLimitWithClearance();
		maxPosition = getHighPositionLimitWithClearance();
		positionProgress = getPositionProgress();
		*/

		if (actualPosition_axisUnits < minPosition || actualPosition_axisUnits > maxPosition) {
			if (actualPosition_axisUnits < getLowPositionLimit() || actualPosition_axisUnits > getHighPositionLimit()) {
				sprintf(positionString, "Axis out of limits : %.2f %s", actualPosition_axisUnits, getPositionUnit(positionUnit)->shortForm);
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
			}
			else {
				sprintf(positionString, "%.2f %s", actualPosition_axisUnits, getPositionUnit(positionUnit)->shortForm);
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
			}
		}
		else {
			sprintf(positionString, "%.2f %s", actualPosition_axisUnits, getPositionUnit(positionUnit)->shortForm);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		}
	}
	ImGui::Text("Current Position : (in range from %.2f %s to %.2f %s)", minPosition, getPositionUnitStringShort(positionUnit), maxPosition, getPositionUnitStringShort(positionUnit));
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
		velocityProgress = std::abs(actualVelocity_axisUnitsPerSecond) / velocityLimit_axisUnitsPerSecond;
		sprintf(velocityString, "%.2f %s/s", actualVelocity_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));
		if (std::abs(actualVelocity_axisUnitsPerSecond) > std::abs(velocityLimit_axisUnitsPerSecond))
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}

	ImGui::Text("Current Velocity : (max %.2f%s/s)", velocityLimit_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));
	ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), velocityString);
	ImGui::PopStyleColor();

	//if (b_manualControlsEnabled) {
		//target movement progress
	float targetProgress;
	double movementSecondsLeft = 0.0;
	static char movementProgressChar[8];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "Machine Disabled");
		targetProgress = 1.0;
	}
	else if (controlMode != ControlMode::Mode::POSITION_TARGET) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "No Target Movement");
		targetProgress = 1.0;
	}
	else if (targetInterpolation->getProgressAtTime(profileTime_seconds) >= 1.0) {
		targetProgress = 1.0;
		sprintf(movementProgressChar, "Movement Finished");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	else {
		targetProgress = targetInterpolation->getProgressAtTime(profileTime_seconds);
		movementSecondsLeft = targetInterpolation->outTime - profileTime_seconds;
		if (movementSecondsLeft < 0.0) movementSecondsLeft = 0.0;
		sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
	}
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), movementProgressChar);
	ImGui::PopStyleColor();
	//}

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
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
			double distanceOutsideRange = range > 1.0 ? feedbackPosition_deviceUnits - rangeMax_deviceUnits : feedbackPosition_deviceUnits - rangeMin_deviceUnits;
			sprintf(rangeString, "Encoder Outside Working Range by %.2f%s", distanceOutsideRange, getPositionUnit(devicePositionUnit)->shortForm);
			range = 1.0;
		}
		ImGui::Text("Feedback Position in Working Range : (%.2f%s to %.2f%s)", rangeMin_deviceUnits, getPositionUnit(devicePositionUnit)->shortForm, rangeMax_deviceUnits, getPositionUnit(devicePositionUnit)->shortForm);
		ImGui::ProgressBar(range, glm::vec2(widgetWidth, ImGui::GetTextLineHeightWithSpacing()), rangeString);
		ImGui::PopStyleColor();
	}

}

float PositionControlledAxis::getFeedbackGuiHeight() {
	return ImGui::GetTextLineHeight() * 13.0;
}



void PositionControlledAxis::setupGui() {

	float widgetWidth = ImGui::GetContentRegionAvail().x;
	float tripleWidgetWidth = (widgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);

	//if (b_manualControlsEnabled) {

	ImGui::Separator();

	//-------------------------- HOMING CONTROLS ---------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Origin and Limit Setting");
	ImGui::PopFont();

	ImGui::SameLine();
	if (beginHelpMarker("(help)")) {
		switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Signal At Lower Limit");
			ImGui::PopFont();
			ImGui::TextWrapped("The Homing Sequence will move the axis in the negative direction until the negative limit signal is triggered."
				"\nThe Axis Origin will be set at the negative limit signal."
				"\nThe Positive Limit is set by manually moving the axis to the desired position and capturing it as the limit.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Signal At Lower and Upper Limit");
			ImGui::PopFont();
			ImGui::TextWrapped("The Homing Sequence will first move the axis in the %s direction, then in the %s direction until each limit signal is triggered."
				"\nThe Axis Origin will be set at the negative limit signal."
				"\nThe Positive Limit will be set at the positive limit signal.",
				homingDirection == HomingDirection::Type::NEGATIVE ? "Negative" : "Positive",
				homingDirection == HomingDirection::Type::NEGATIVE ? "Positive" : "Negative");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Signal At Origin");
			ImGui::PopFont();
			ImGui::TextWrapped("The Homing Sequence will move the axis in the %s direction until the reference signal is triggered."
				"\nThe Axis Origin will be set at the reference signal."
				"\nThe Positive and Negative limits can be set by manually moving the axis to the desired limits and capturing the positions.",
				homingDirection == HomingDirection::Type::NEGATIVE ? "Negative" : "Positive");
			break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("No Position Reference Signal");
			ImGui::PopFont();
			ImGui::TextWrapped("No Homing Sequence is available."
				"\nThe Axis origin and limits have to be set manually.");
			break;
		}
		endHelpMarker();
	}

	glm::vec2 homingButtonSize((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 1.5);

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN: {
		bool homing = isHoming();
		if (homing) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Button("Start Homing", homingButtonSize)) startHoming();
		if (homing) END_DISABLE_IMGUI_ELEMENT
			ImGui::SameLine();
		if (!homing) BEGIN_DISABLE_IMGUI_ELEMENT
			if (ImGui::Button("Stop Homing", homingButtonSize)) cancelHoming();
		if (!homing) END_DISABLE_IMGUI_ELEMENT

			static char homingStatusString[128];
		glm::vec2 homingStatusSize(widgetWidth, ImGui::GetTextLineHeight() * 1.5);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		if (isHoming()) {
			sprintf(homingStatusString, "Homing Status: %s", getHomingStep(homingStep)->displayName);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		}
		else if (didHomingSucceed()) {
			sprintf(homingStatusString, "Homing Succeeded");
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		}
		else if (didHomingFail()) {
			sprintf(homingStatusString, "Homing Failed: %s", getHomingError(homingError)->displayName);
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		}
		else {
			sprintf(homingStatusString, "No Homing Ongoing");
			ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
		}
		ImGui::Button(homingStatusString, homingStatusSize);
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		break;
	}

	glm::vec2 singleButtonSize(widgetWidth, tripleButtonSize.y);
	glm::vec2 doubleButtonSize((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, tripleButtonSize.y);

	bool disableCaptureButtons = !isEnabled() || isMoving();
	bool disableCaptureLowerLimit = actualPosition_axisUnits > 0.0;
	bool disableCaptureHigherLimit = actualPosition_axisUnits < 0.0;
	if (disableCaptureButtons) BEGIN_DISABLE_IMGUI_ELEMENT
		switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			if (disableCaptureHigherLimit) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Capture Positive Limit", singleButtonSize)) setCurrentPositionAsPositiveLimit();
			if (disableCaptureHigherLimit) END_DISABLE_IMGUI_ELEMENT
				break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			if (disableCaptureLowerLimit) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Capture Negative Limit", doubleButtonSize)) setCurrentPositionAsNegativeLimit();
			if (disableCaptureLowerLimit) END_DISABLE_IMGUI_ELEMENT
				ImGui::SameLine();
			if (disableCaptureHigherLimit) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Capture Positive Limit", doubleButtonSize)) setCurrentPositionAsPositiveLimit();
			if (disableCaptureHigherLimit) END_DISABLE_IMGUI_ELEMENT
				break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			if (disableCaptureLowerLimit) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Capture Negative Limit", tripleButtonSize)) setCurrentPositionAsNegativeLimit();
			if (disableCaptureLowerLimit) END_DISABLE_IMGUI_ELEMENT
				ImGui::SameLine();
			if (ImGui::Button("Capture Origin", tripleButtonSize)) setCurrentPosition(0.0);
			ImGui::SameLine();
			if (disableCaptureHigherLimit) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::Button("Capture Positive Limit", tripleButtonSize)) setCurrentPositionAsPositiveLimit();
			if (disableCaptureHigherLimit) END_DISABLE_IMGUI_ELEMENT
				break;
		}
	if (disableCaptureButtons) END_DISABLE_IMGUI_ELEMENT


		ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Feedback Scaling");
	ImGui::PopFont();

	ImGui::SameLine();
	if (beginHelpMarker("(help)")) {

		ImGui::TextWrapped("This utility allows the setting of feedback to axis unit conversion ratio."
			"\nMove the axis to its origin (0.0) and physically mark down the position of the axis."
			"\nMove the axis to another position and measure the distance to the initial mark."
			"\nEnter the measured distance into the utility and click the \"Set Scaling\" Button."
			"\nThe display will now match the actual position and the conversion ratio will be set correctly."
			"\nLarger and more precise measured distance will yield a greater precision in the ratio.");

		endHelpMarker();
	}

	static char scalingString[64];
	sprintf(scalingString, "%.4f %s", machineScalingPosition_axisUnits, getPositionUnit(positionUnit)->shortForm);

	ImGui::TextWrapped("Current Axis Position Relative to the origin");
	ImGui::SetNextItemWidth(doubleButtonSize.x);
	ImGui::InputDouble("##posScal", &machineScalingPosition_axisUnits, 0.0, 0.0, scalingString);
	ImGui::SameLine();

	bool disableSetScaling = (machineScalingPosition_axisUnits > 0.0 && actualPosition_axisUnits < 0.0)
		|| (machineScalingPosition_axisUnits < 0.0 && actualPosition_axisUnits > 0.0)
		|| machineScalingPosition_axisUnits == 0.0
		|| isMoving();
	if (disableSetScaling) BEGIN_DISABLE_IMGUI_ELEMENT
		if (ImGui::Button("Set Scaling", ImGui::GetItemRectSize())) scaleFeedbackToMatchPosition(machineScalingPosition_axisUnits);
	if (disableSetScaling) END_DISABLE_IMGUI_ELEMENT
		//}
}





void PositionControlledAxis::settingsGui() {

	//------------------ GENERAL MACHINE SETTINGS -------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();

	ImGui::Text("Movement Type :");
	if (ImGui::BeginCombo("##AxisUnitType", getPositionUnitType(positionUnitType)->displayName)) {
		for (PositionUnitType& unitType : getPositionUnitTypes()) {
			if (ImGui::Selectable(unitType.displayName, positionUnitType == unitType.type)) {
				positionUnitType = unitType.type;
				//if the machine type is changed but the machine unit is of the wrong type
				//change the machine unit to the first correct type automatically
				if (getPositionUnit(positionUnit)->type != unitType.type) {
					switch (unitType.type) {
					case PositionUnit::Type::ANGULAR:
						positionUnit = getAngularPositionUnits().front().unit;
						positionReferenceSignal = getAngularPositionReferenceSignals().front().type;
						break;
					case PositionUnit::Type::LINEAR:
						positionUnit = getLinearPositionUnits().front().unit;
						positionReferenceSignal = getLinearPositionReferenceSignals().front().type;
						break;
					}
				}
			}
		}
		ImGui::EndCombo();
	}

	float widgetWidth = ImGui::GetItemRectSize().x;

	ImGui::Text("Position Unit :");
	if (ImGui::BeginCombo("##AxisUnit", getPositionUnit(positionUnit)->displayName)) {
		if (positionUnitType == PositionUnit::Type::LINEAR) {
			for (PositionUnit& unit : getLinearPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, positionUnit == unit.unit)) positionUnit = unit.unit;
			}
		}
		else if (positionUnitType == PositionUnit::Type::ANGULAR) {
			for (PositionUnit& unit : getAngularPositionUnits()) {
				if (ImGui::Selectable(unit.displayName, positionUnit == unit.unit)) positionUnit = unit.unit;
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
				ImGui::Text("%s %s per Machine %s :", feedbackDevice->getName(), getPositionUnit(feedbackDevice->positionUnit)->displayNamePlural, getPositionUnit(positionUnit)->displayName);
			ImGui::InputDouble("##feedbackCoupling", &feedbackUnitsPerAxisUnits);
			if (feedbackUnitsPerAxisUnits < 0.0) feedbackUnitsPerAxisUnits = 0.0;
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

			ImGui::Text("%s %s per Machine %s :", actuator->getName(), getPositionUnit(actuator->positionUnit)->displayNamePlural, getPositionUnit(positionUnit)->displayName);
			ImGui::InputDouble("##actuatorCoupling", &actuatorUnitsPerAxisUnits);

			ImGui::Checkbox("##actFeedIdent", &feedbackAndActuatorConversionIdentical);
			ImGui::SameLine();
			ImGui::Text("Actuator and Feedback Coupling are identical");

			if (actuatorUnitsPerAxisUnits < 0.0) actuatorUnitsPerAxisUnits = 0.0;
			if (feedbackAndActuatorConversionIdentical) feedbackUnitsPerAxisUnits = actuatorUnitsPerAxisUnits;
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

			ImGui::Text("%s %s per Machine %s :", servo->getName(), getPositionUnit(servo->positionUnit)->displayNamePlural, getPositionUnit(positionUnit)->displayName);
			ImGui::InputDouble("##servoActuatorCoupling", &actuatorUnitsPerAxisUnits);
			if (actuatorUnitsPerAxisUnits < 0.0) actuatorUnitsPerAxisUnits = 0.0;
			feedbackUnitsPerAxisUnits = actuatorUnitsPerAxisUnits;

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
		double actuatorVelocityLimit_axisUnitsPerSecond;
		double actuatorAccelerationLimit_axisUnitsPerSecondSquared;
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
		actuatorVelocityLimit_axisUnitsPerSecond = actuatorVelocityLimit_actuatorUnitsPerSecond / actuatorUnitsPerAxisUnits;
		actuatorAccelerationLimit_axisUnitsPerSecondSquared = actuatorAccelerationLimit_actuatorUnitsPerSecondSquared / actuatorUnitsPerAxisUnits;
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::TextWrapped("Max actuator velocity is %.1f %s/s and max acceleration is %.1f %s/s\xc2\xb2",
			actuatorVelocityLimit_actuatorUnitsPerSecond,
			getPositionUnit(actuatorUnit)->shortForm,
			actuatorAccelerationLimit_actuatorUnitsPerSecondSquared,
			getPositionUnit(actuatorUnit)->shortForm);
		ImGui::TextWrapped("Machine is limited to %.3f %s/s and %.3f %s/s\xc2\xb2",
			actuatorVelocityLimit_axisUnitsPerSecond,
			getPositionUnit(positionUnit)->shortForm,
			actuatorAccelerationLimit_axisUnitsPerSecondSquared,
			getPositionUnit(positionUnit)->shortForm);
		ImGui::PopStyleColor();
		clampValue(velocityLimit_axisUnitsPerSecond, 0.0, actuatorVelocityLimit_axisUnitsPerSecond);
		clampValue(accelerationLimit_axisUnitsPerSecondSquared, 0.0, actuatorAccelerationLimit_axisUnitsPerSecondSquared);
		clampValue(defaultManualVelocity_axisUnitsPerSecond, 0.0, actuatorVelocityLimit_axisUnitsPerSecond);
		clampValue(defaultManualAcceleration_axisUnitsPerSecondSquared, 0.0, actuatorAccelerationLimit_axisUnitsPerSecondSquared);
	}

	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", velocityLimit_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));
	ImGui::InputDouble("##VelLimit", &velocityLimit_axisUnitsPerSecond, 0.0, 0.0, velLimitString);
	static char accLimitString[16];
	sprintf(accLimitString, "%.3f %s/s\xc2\xb2", accelerationLimit_axisUnitsPerSecondSquared, getPositionUnitStringShort(positionUnit));
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &accelerationLimit_axisUnitsPerSecondSquared, 0.0, 0.0, accLimitString);

	double halfWidgetWidth = (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;

	ImGui::Text("Default Manual Control Parameters", getPositionUnitStringPlural(positionUnit));
	ImGui::SetNextItemWidth(halfWidgetWidth);
	static char manAccString[16];
	sprintf(manAccString, "%.3f %s/s\xc2\xb2", defaultManualAcceleration_axisUnitsPerSecondSquared, getPositionUnitStringShort(positionUnit));
	ImGui::InputDouble("##defmanAcc", &defaultManualAcceleration_axisUnitsPerSecondSquared, 0.0, 0.0, manAccString);
	clampValue(defaultManualAcceleration_axisUnitsPerSecondSquared, 0.0, accelerationLimit_axisUnitsPerSecondSquared);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(halfWidgetWidth);
	static char manVelString[16];
	sprintf(manVelString, "%.3f %s/s", defaultManualVelocity_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));
	ImGui::InputDouble("##defmanvel", &defaultManualVelocity_axisUnitsPerSecond, 0.0, 0.0, manVelString);
	clampValue(defaultManualVelocity_axisUnitsPerSecond, 0.0, velocityLimit_axisUnitsPerSecond);

	ImGui::Separator();

	//----------------- REFERENCE SIGNALS, ORIGN AND HOMING -----------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Reference Signals & Homing");
	ImGui::PopFont();

	static auto showPositionReferenceDescription = [](PositionReferenceSignal::Type type) {
		switch (type) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			ImGui::TextWrapped("Single Limit Signal at the negative end of the machine travel."
				"\nHoming will move the machine in the negative direction"
				"\nAxis Origin is set at the low limit.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			ImGui::TextWrapped("Two Limit Signals at each end of the axis travel."
				"\nHoming will first move the axis in the specified direction, then in the other direction"
				"\nAxis Origin is set at the low limit.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			ImGui::TextWrapped("Single Reference Signal inside the axis travel range."
				"\nHoming will find the signal using the specified direction."
				"\nAxis Origin is set at the reference signal.");
			break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			ImGui::TextWrapped("No reference Signal, the machine is positionned using only feedback data."
				"\nAxis origin is set by manually moving the machine to the desired position and capturing the origin.");
			break;
		}
	};

	ImGui::Text("Reference Signal Type");
	if (ImGui::BeginCombo("##MovementType", getPositionReferenceSignal(positionReferenceSignal)->displayName)) {
		switch (positionUnitType) {
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


	if (needsReferenceDevice()) {
		if (!isReferenceDeviceConnected()) {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("No Reference Device connected.");
			ImGui::PopStyleColor();
		}
		else {
			std::shared_ptr<GpioDevice> gpioDevice = getReferenceDevice();
			ImGui::PushFont(Fonts::robotoBold15);
			ImGui::Text("Reference Device:");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("%s on %s", gpioDevice->getName(), gpioDevice->parentDevice->getName());
		}

		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			if (!lowLimitSignalPin->isConnected()) ImGui::TextWrapped("No Negative Limit Signal Connected.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			if (!lowLimitSignalPin->isConnected()) ImGui::TextWrapped("No Negative Limit Signal Connected.");
			if (!highLimitSignalPin->isConnected()) ImGui::TextWrapped("No Positive Limit Signal Connected.");
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			if (!referenceSignalPin->isConnected()) ImGui::TextWrapped("No Reference Signal Connected.");
			break;
		}
		ImGui::PopStyleColor();

		static char homVelString[16];
		sprintf(homVelString, "%.3f %s/s", homingVelocity_axisUnitsPerSecond, getPositionUnitStringShort(positionUnit));

		switch (positionReferenceSignal) {
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
			ImGui::Text("Homing Velocity", getPositionUnit(positionUnit)->displayNamePlural);
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_axisUnitsPerSecond, 0.0, 0.0, homVelString);
			if (homingVelocity_axisUnitsPerSecond < 0) homingVelocity_axisUnitsPerSecond = abs(homingVelocity_axisUnitsPerSecond);
			break;
		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
			ImGui::Text("Homing Direction");
			if (ImGui::BeginCombo("##HomingDirection", getHomingDirection(homingDirection)->displayName)) {
				for (HomingDirection& direction : getHomingDirections()) {
					bool selected = homingDirection == direction.type;
					if (ImGui::Selectable(direction.displayName, selected)) homingDirection = direction.type;
				}
				ImGui::EndCombo();
			}
			ImGui::Text("Homing Velocity", getPositionUnit(positionUnit)->displayNamePlural);
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_axisUnitsPerSecond, 0.0, 0.0, homVelString);
			if (homingVelocity_axisUnitsPerSecond < 0) homingVelocity_axisUnitsPerSecond = abs(homingVelocity_axisUnitsPerSecond);
			break;
		case PositionReferenceSignal::Type::NO_SIGNAL:
			break;
		}


	}

	ImGui::Separator();

	//----------------- POSITION LIMITS -----------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Position Limits");
	ImGui::PopFont();

	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		ImGui::TextWrapped("Axis is limited between Lower Limit Signal (Origin) and an Upper Limit Parameter.");
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		ImGui::TextWrapped("Axis is limited between Lower and Upper Limit Signals.");
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		ImGui::TextWrapped("Axis is limited between Lower and Upper Limit Parameters.");
		break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		ImGui::TextWrapped("Axis is limited between Lower and Upper Limit Parameters.");
		break;
	}
	ImGui::PopStyleColor();

	static char negDevString[16];
	sprintf(negDevString, "%.3f %s", maxNegativeDeviation_axisUnits, getPositionUnitStringShort(positionUnit));
	static char posDevString[16];
	sprintf(posDevString, "%.3f %s", maxPositiveDeviation_axisUnits, getPositionUnitStringShort(positionUnit));

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		ImGui::Text("Max Distance From Origin");
		ImGui::SetNextItemWidth(halfWidgetWidth);
		ImGui::InputDouble("##MaxDeviation", &maxPositiveDeviation_axisUnits, 0.0, 0.0, posDevString);
		if (maxPositiveDeviation_axisUnits < 0.0) maxPositiveDeviation_axisUnits = 0.0;
		ImGui::SameLine();
		ImGui::Checkbox("##enableUpperLimit", &enablePositiveLimit);
		ImGui::SameLine();
		ImGui::Text("Enable Upper Axis Limit");
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
	case PositionReferenceSignal::Type::NO_SIGNAL:
		ImGui::Text("Max Positive Distance From Origin");
		ImGui::SetNextItemWidth(halfWidgetWidth);
		ImGui::InputDouble("##MaxPositiveDeviation", &maxPositiveDeviation_axisUnits, 0.0, 0.0, posDevString);
		if (maxPositiveDeviation_axisUnits < 0.0) maxPositiveDeviation_axisUnits = 0.0;
		ImGui::SameLine();
		ImGui::Checkbox("##enableUpperLimit", &enablePositiveLimit);
		ImGui::SameLine();
		ImGui::Text("Enable Upper Axis Limit");
		ImGui::Text("Max Negative Distance From Origin");
		ImGui::SetNextItemWidth(halfWidgetWidth);
		ImGui::InputDouble("##MaxNegativeDeviation", &maxNegativeDeviation_axisUnits, 0.0, 0.0, negDevString);
		if (maxNegativeDeviation_axisUnits > 0.0) maxNegativeDeviation_axisUnits = 0.0;
		ImGui::SameLine();
		ImGui::Checkbox("##enableLowerLimit", &enableNegativeLimit);
		ImGui::SameLine();
		ImGui::Text("Enable Lower Axis Limit");
		break;
	}

	static char clearanceString[16];
	sprintf(clearanceString, "%.3f %s", limitClearance_axisUnits, getPositionUnitStringShort(positionUnit));
	ImGui::Text("Limit Clearance");
	ImGui::InputDouble("##limitclearance", &limitClearance_axisUnits, 0.0, 0.0, clearanceString);

	ImGui::Checkbox("##limitToFeedbackRange", &limitToFeedbackWorkingRange);
	ImGui::SameLine();
	ImGui::Text("Limit Axis to Position Feedback Working Range");

	ImGui::Text("Axis Is Limited between %.3f %s and %.3f %s",
		getLowPositionLimit(),
		getPositionUnit(positionUnit)->displayNamePlural,
		getHighPositionLimit(),
		getPositionUnit(positionUnit)->displayNamePlural);

}








void PositionControlledAxis::devicesGui() {

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
		ImGui::Separator();
	}

	if (needsActuatorDevice()) {
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
		ImGui::Separator();
	}
	else if (needsServoActuatorDevice()) {
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
		ImGui::Separator();
	}

	if (needsReferenceDevice()) {
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





void PositionControlledAxis::metricsGui() {

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
	float maxV = velocityLimit_axisUnitsPerSecond;
	float minV = -velocityLimit_axisUnitsPerSecond;
	float minA = -accelerationLimit_axisUnitsPerSecondSquared;
	float maxA = accelerationLimit_axisUnitsPerSecondSquared;

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
