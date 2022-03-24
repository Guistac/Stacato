#include <pch.h>

#include "Motion/Axis/PositionControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Environnement/DeviceNode.h"
#include "Gui/Utilities/HelpMarker.h"

#include "Motion/SubDevice.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/Curve/Curve.h"

void PositionControlledAxis::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Control")) {
		ImGui::BeginChild("ControlsChild");
		controlsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		ImGui::BeginChild("Settings");
		settingsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		ImGui::BeginChild("Devices");
		devicesGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		ImGui::BeginChild("Metrics");
		metricsGui();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}




void PositionControlledAxis::controlsGui() {
	
	float singleWidgetWidth = ImGui::GetContentRegionAvail().x;
	float tripleWidgetWidth = (singleWidgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
	float doubleWidgetWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	glm::vec2 singleButtonSize(singleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	glm::vec2 doubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetTextLineHeight() * 1.5);
	glm::vec2 largeDoubleButtonSize(doubleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);
	glm::vec2 progressBarSize(singleWidgetWidth, ImGui::GetFrameHeight());
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	//------------------- STATE CONTROLS ------------------------
	
	bool axisIsControlledExternally = isAxisPinConnected();
	if (axisIsControlledExternally) {
		ImGui::TextWrapped("Axis is Controlled by Node '%s'."
						   "\nManual controls are disabled.",
						   axisPin->getConnectedPin()->getNode()->getName());
	}
	ImGui::BeginDisabled(axisIsControlledExternally);
	
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Enabled", largeDoubleButtonSize);
	}
	else if (isReady()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Ready", largeDoubleButtonSize);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Disabled", largeDoubleButtonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();

	ImGui::SameLine();
	if (isEnabled()) {
		if (ImGui::Button("Disable Axis", largeDoubleButtonSize)) disable();
	}
	else if (isReady()){
		if (ImGui::Button("Enable Axis", largeDoubleButtonSize)) enable();
	}else{
		ImGui::Button("Axis Not Ready", largeDoubleButtonSize);
	}
	ImGui::EndDisabled();
	
	bool b_disableControls = axisIsControlledExternally || !isEnabled();
	ImGui::BeginDisabled(b_disableControls);
		
	//------------------- VELOCITY CONTROLS ------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Velocity Control");
	ImGui::PopFont();

	static char velocityTargetString[32];
	sprintf(velocityTargetString, "%.3f %s/s", manualVelocityTargetDisplay, Unit::getAbbreviatedString(positionUnit));
	ImGui::SetNextItemWidth(singleWidgetWidth);
	if (ImGui::SliderFloat("##Velocity", &manualVelocityTargetDisplay, -velocityLimit, velocityLimit, velocityTargetString));
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityTargetDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setVelocityTarget(0.0);
		manualVelocityTargetDisplay = 0.0;
	}

	ImGui::Separator();

	//------------------------- POSITION CONTROLS --------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Position Control");
	ImGui::PopFont();

	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", interpolationPositionTarget, Unit::getAbbreviatedString(positionUnit));
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetPosition", &interpolationPositionTarget, 0.0, 0.0, targetPositionString);
	
	ImGui::SameLine();
	static char targetVelocityString[32];
	sprintf(targetVelocityString, "%.3f %s/s", interpolationVelocityTarget, Unit::getAbbreviatedString(positionUnit));
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetVelocity", &interpolationVelocityTarget, 0.0, 0.0, targetVelocityString);
	
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputDouble("##TargetTime", &interpolationTimeTarget, 0.0, 0.0, "%.3f s");
	if (ImGui::Button("Fast Move", tripleButtonSize)) moveToPositionWithVelocity(interpolationPositionTarget, velocityLimit);
	
	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize)) moveToPositionWithVelocity(interpolationPositionTarget, interpolationVelocityTarget);
	
	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize)) moveToPositionInTime(interpolationPositionTarget, interpolationTimeTarget);

	if (ImGui::Button("Stop", largeDoubleButtonSize)) setVelocityTarget(0.0);
	
	ImGui::SameLine();
	if (ImGui::Button("Fast Stop", largeDoubleButtonSize)) fastStop();
	
	

	//-------------------------- HOMING CONTROLS ---------------------------

	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Origin and Limit Setting");
	ImGui::PopFont();

	ImGui::SameLine();
	if (beginHelpMarker("(help)")) {
		switch (positionReferenceSignal) {
			case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::Text("Signal At Lower Limit");
				ImGui::PopFont();
				ImGui::TextWrapped("The Homing Sequence will move the axis in the negative direction until the negative limit signal is triggered."
					"\nThe Axis Origin will be set at the negative limit signal."
					"\nThe Positive Limit is set by manually moving the axis to the desired position and capturing it as the limit.");
				break;
			case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::Text("Signal At Lower and Upper Limit");
				ImGui::PopFont();
				ImGui::TextWrapped("The Homing Sequence will first move the axis in the %s direction, then in the %s direction until each limit signal is triggered."
					"\nThe Axis Origin will be set at the negative limit signal."
					"\nThe Positive Limit will be set at the positive limit signal.",
					homingDirection == HomingDirection::NEGATIVE ? "Negative" : "Positive",
					homingDirection == HomingDirection::NEGATIVE ? "Positive" : "Negative");
				break;
			case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::Text("Signal At Origin");
				ImGui::PopFont();
				ImGui::TextWrapped("The Homing Sequence will move the axis in the %s direction until the reference signal is triggered."
					"\nThe Axis Origin will be set at the reference signal."
					"\nThe Positive and Negative limits can be set by manually moving the axis to the desired limits and capturing the positions.",
					homingDirection == HomingDirection::NEGATIVE ? "Negative" : "Positive");
				break;
			case PositionReferenceSignal::NO_SIGNAL:
				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::Text("No Position Reference Signal");
				ImGui::PopFont();
				ImGui::TextWrapped("No Homing Sequence is available."
					"\nThe Axis origin and limits have to be set manually.");
				break;
		}
		endHelpMarker();
	}

	static char homingStatusString[128];
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			if(isHoming()){
				if (ImGui::Button("Stop Homing", singleButtonSize)) cancelHoming();
			}else{
				if (ImGui::Button("Start Homing", singleButtonSize)) startHoming();
			}
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			if (isHoming()) {
				sprintf(homingStatusString, "Homing Status: %s", Enumerator::getDisplayString(homingStep));
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
			}
			else if (didHomingSucceed()) {
				sprintf(homingStatusString, "Homing Succeeded");
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
			}
			else if (didHomingFail()) {
				sprintf(homingStatusString, "Homing Failed: %s", Enumerator::getDisplayString(homingError));
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
			}
			else {
				sprintf(homingStatusString, "No Homing Ongoing");
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
			}
			ImGui::Button(homingStatusString, singleButtonSize);
			ImGui::PopItemFlag();
			ImGui::PopStyleColor();
		default: break;
	}

	bool b_disableCaptureButtons = !isEnabled() || isMoving();
	bool disableCaptureLowerLimit = *actualPositionValue > 0.0;
	bool disableCaptureHigherLimit = *actualPositionValue < 0.0;
	ImGui::BeginDisabled(b_disableCaptureButtons);
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			ImGui::BeginDisabled(disableCaptureHigherLimit);
			if (ImGui::Button("Capture Positive Limit", singleButtonSize)) setCurrentPositionAsPositiveLimit();
			ImGui::EndDisabled();
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			ImGui::BeginDisabled(disableCaptureLowerLimit);
			if (ImGui::Button("Capture Negative Limit", doubleButtonSize)) setCurrentPositionAsNegativeLimit();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(disableCaptureHigherLimit);
			if (ImGui::Button("Capture Positive Limit", doubleButtonSize)) setCurrentPositionAsPositiveLimit();
			ImGui::EndDisabled();
			break;
		case PositionReferenceSignal::NO_SIGNAL:
			ImGui::BeginDisabled(disableCaptureLowerLimit);
			if (ImGui::Button("Capture Negative Limit", tripleButtonSize)) setCurrentPositionAsNegativeLimit();
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Capture Origin", tripleButtonSize)) setCurrentPosition(0.0);
			ImGui::SameLine();
			ImGui::BeginDisabled(disableCaptureHigherLimit);
			if (ImGui::Button("Capture Positive Limit", tripleButtonSize)) setCurrentPositionAsPositiveLimit();
			ImGui::EndDisabled();
			break;
		default: break;
	}
	ImGui::EndDisabled();

	//-------------------------- POSITION FEEDBACK SCALING ---------------------------
		
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

	ImGui::TextWrapped("Current Axis Position Relative to the origin");
	static char scalingString[64];
	sprintf(scalingString, "%.4f %s", axisScalingPosition, Unit::getAbbreviatedString(positionUnit));
	ImGui::SetNextItemWidth(doubleButtonSize.x);
	ImGui::InputDouble("##posScal", &axisScalingPosition, 0.0, 0.0, scalingString);
	
	ImGui::SameLine();
	bool disableSetScaling = (axisScalingPosition > 0.0 && *actualPositionValue < 0.0)
		|| (axisScalingPosition < 0.0 && *actualPositionValue > 0.0)
		|| axisScalingPosition == 0.0
		|| isMoving();
	ImGui::BeginDisabled(disableSetScaling);
	if (ImGui::Button("Set Scaling", ImGui::GetItemRectSize())) scaleFeedbackToMatchPosition(axisScalingPosition);
	ImGui::EndDisabled();
	
		
		
		
	//-------------------------------- FEEDBACK --------------------------------
		
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Feedback");
	ImGui::PopFont();

	
	//actual position in range
	double minPosition = 0.0;
	double maxPosition = 0.0;
	double positionProgress = 0.0;
	static char positionString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		positionProgress = 1.0;
		sprintf(positionString, "Axis Disabled");
	}
	else {
		minPosition = getLowPositionLimit();
		maxPosition = getHighPositionLimit();
		positionProgress = getActualPositionNormalized();
		if (*actualPositionValue < minPosition || *actualPositionValue > maxPosition) {
			if (*actualPositionValue < getLowPositionLimitWithoutClearance() || *actualPositionValue > getHighPositionLimitWithoutClearance()) {
				sprintf(positionString, "Axis out of limits : %.2f %s", *actualPositionValue, Unit::getAbbreviatedString(positionUnit));
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
			}
			else {
				sprintf(positionString, "%.2f %s", *actualPositionValue, Unit::getAbbreviatedString(positionUnit));
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
			}
		}
		else {
			sprintf(positionString, "%.2f %s", *actualPositionValue, Unit::getAbbreviatedString(positionUnit));
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		}
	}
	const char* shortPositionUnitString = Unit::getAbbreviatedString(positionUnit);
	ImGui::Text("Current Position : (in range from %.2f %s to %.2f %s)", minPosition, shortPositionUnitString, maxPosition, shortPositionUnitString);
	ImGui::ProgressBar(positionProgress, progressBarSize, positionString);
	ImGui::PopStyleColor();


	//actual velocity
	float velocityProgress;
	static char velocityString[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		velocityProgress = 1.0;
		sprintf(velocityString, "Axis Disabled");
	}
	else {
		velocityProgress = std::abs(*actualVelocityValue) / velocityLimit;
		sprintf(velocityString, "%.2f %s/s", *actualVelocityValue, Unit::getAbbreviatedString(positionUnit));
		if (std::abs(*actualVelocityValue) > std::abs(velocityLimit))
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	ImGui::Text("Current Velocity : (max %.2f%s/s)", velocityLimit, Unit::getAbbreviatedString(positionUnit));
	ImGui::ProgressBar(velocityProgress, progressBarSize, velocityString);
	ImGui::PopStyleColor();

	float positionErrorProgress;
	float maxfollowingError = 0.0;
	static char positionErrorString[32];
	if(!isEnabled()){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		positionErrorProgress = 1.0;
		sprintf(positionErrorString, "Axis Disabled");
	}else{
		auto servoActuator = getServoActuatorDevice();
		positionErrorProgress = std::abs(servoActuator->getFollowingErrorNormalized());
		if(positionErrorProgress < 1.0) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		maxfollowingError = servoActuatorUnitsToAxisUnits(servoActuator->maxfollowingError);
		double followingError = servoActuatorUnitsToAxisUnits(servoActuator->getFollowingError());
		sprintf(positionErrorString, "%.3f %s", followingError, Unit::getAbbreviatedString(positionUnit));
	}
	
	ImGui::Text("Current Following Error : (max %.3f%s)", maxfollowingError, Unit::getAbbreviatedString(positionUnit));
	ImGui::ProgressBar(positionErrorProgress, progressBarSize, positionErrorString);
	ImGui::PopStyleColor();
	
	//target movement progress
	float targetProgress;
	double movementSecondsLeft = 0.0;
	static char movementProgressChar[32];
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "Axis Disabled");
		targetProgress = 1.0;
	}
	else if (controlMode != ControlMode::POSITION_TARGET) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(movementProgressChar, "No Target Movement");
		targetProgress = 1.0;
	}
	else if (motionProfile.isInterpolationFinished(profileTime_seconds)) {
		targetProgress = 1.0;
		sprintf(movementProgressChar, "Movement Finished");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
	}
	else {
		targetProgress = motionProfile.getInterpolationProgress(profileTime_seconds);
		movementSecondsLeft = motionProfile.getRemainingInterpolationTime(profileTime_seconds);
		sprintf(movementProgressChar, "%.2fs", movementSecondsLeft);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
	}
	ImGui::Text("Movement Time Remaining :");
	ImGui::ProgressBar(targetProgress, progressBarSize, movementProgressChar);
	ImGui::PopStyleColor();
	
	
	//encoder position in working range
	double rangeProgress = 1.0;
	double feedbackPosition = 0.0;
	double rangeMin = std::numeric_limits<double>::quiet_NaN();
	double rangeMax = std::numeric_limits<double>::quiet_NaN();
	Unit::Distance feedbackPositionUnit;
	static char rangeString[64];
	if (isServoActuatorDeviceConnected()) {
		std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
		rangeProgress = servo->getPositionInRange();
		rangeMin = servo->getMinPosition();
		rangeMax = servo->getMaxPosition();
		feedbackPositionUnit = servo->positionUnit;
		feedbackPosition = servo->getPosition();
	}
	if (!isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::blue);
		sprintf(rangeString, "Axis Disabled");
		rangeProgress = 1.0;
	}
	else if (rangeProgress < 1.0 && rangeProgress > 0.0) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		sprintf(rangeString, "%.2f%s", feedbackPosition, Unit::getAbbreviatedString(feedbackPositionUnit));
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (int)(1000 * Timing::getProgramTime_seconds()) % 500 > 250 ? Colors::red : Colors::darkRed);
		double distanceOutsideRange = rangeProgress > 1.0 ? feedbackPosition - rangeMax : feedbackPosition - rangeMin;
		sprintf(rangeString, "Encoder Outside Working Range by %.2f%s", distanceOutsideRange, Unit::getAbbreviatedString(feedbackPositionUnit));
		rangeProgress = 1.0;
	}
	ImGui::Text("Feedback Position in Working Range : (%.2f%s to %.2f%s)",
				rangeMin,
				Unit::getAbbreviatedString(feedbackPositionUnit),
				rangeMax,
				Unit::getAbbreviatedString(feedbackPositionUnit));
	ImGui::ProgressBar(rangeProgress, progressBarSize, rangeString);
	ImGui::PopStyleColor();
	
	ImGui::EndDisabled();
}








void PositionControlledAxis::settingsGui() {
	
	//------------------ GENERAL MACHINE SETTINGS -------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();

	ImGui::Text("Movement Type :");
	if (ImGui::BeginCombo("##AxisUnitType", Enumerator::getDisplayString(positionUnitType))) {
		for (auto& type : Enumerator::getTypes<Unit::DistanceType>()) {
			if (ImGui::Selectable(type.displayString, positionUnitType == type.enumerator)) {
				setPositionUnitType(type.enumerator);
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Position Unit :");
	if (ImGui::BeginCombo("##AxisUnit", Unit::getDisplayString(positionUnit))) {
		for(auto& type : Unit::getUnits<Unit::Distance>()){
			switch(positionUnitType){
				case Unit::DistanceType::LINEAR:
					if(!Unit::isLinearDistance(type.enumerator)) continue;
					break;
				case Unit::DistanceType::ANGULAR:
					if(!Unit::isAngularDistance(type.enumerator)) continue;
					break;
			}
			if (ImGui::Selectable(type.displayString, positionUnit == type.enumerator)) setPositionUnit(type.enumerator);
		}
		ImGui::EndCombo();
	}

	//------------------------ SERVO ACTUATOR -----------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Servo Actuator");
	ImGui::PopFont();

	if (isServoActuatorDeviceConnected()) {

		std::shared_ptr<ServoActuatorDevice> servo = getServoActuatorDevice();
		PositionFeedbackType feedbackType = servo->getPositionFeedbackType();
		const char* feedbackTypeString = Enumerator::getDisplayString(feedbackType);
		std::shared_ptr<Device> servoActuatorParentDevice = servo->parentDevice;

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Device:");
		ImGui::PopFont();
		ImGui::SameLine();
		if(servoActuatorParentDevice) ImGui::Text("%s on %s", servo->getName(), servo->parentDevice->getName());
		else ImGui::Text("%s on Node %s", servo->getName(), servoActuatorPin->getConnectedPin()->getNode()->getName());

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Position Unit:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", Unit::getDisplayStringPlural(servo->positionUnit));

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Feedback Type:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", feedbackTypeString);

		switch (servo->feedbackType) {
			case PositionFeedbackType::INCREMENTAL:
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("With incremental position feedback, the homing routine needs to be executed on each system power cycle");
			ImGui::PopStyleColor();
			break;
		default: break;
		}

		ImGui::Text("%s %s per Machine %s :", servo->getName(), Unit::getDisplayStringPlural(servo->positionUnit), Unit::getDisplayString(positionUnit));
		ImGui::InputDouble("##servoActuatorCoupling", &servoActuatorUnitsPerAxisUnits);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();

	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("No Servo Actuator device connected.");
		ImGui::PopStyleColor();
	}

	//-------------------------- KINEMATIC LIMITS ----------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Kinematic Limits");
	ImGui::PopFont();

	if (isServoActuatorDeviceConnected()) {
		std::shared_ptr<ServoActuatorDevice> servoActuator = getServoActuatorDevice();
		Unit::Distance servoActuatorPositionUnit = servoActuator->getPositionUnit();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::TextWrapped("Max actuator velocity is %.1f %s/s and max acceleration is %.1f %s/s\xc2\xb2",
						   servoActuator->getVelocityLimit(),
						   Unit::getAbbreviatedString(servoActuatorPositionUnit),
						   servoActuator->getAccelerationLimit(),
						   Unit::getAbbreviatedString(servoActuatorPositionUnit));
		ImGui::TextWrapped("Min actuator velocity is %.3f %s/s",
						   servoActuator->getMinVelocity(),
						   Unit::getAbbreviatedString(servoActuatorPositionUnit));
		
		const char* machineUnitShortString = Unit::getAbbreviatedString(positionUnit);
		ImGui::TextWrapped("Actuator limits axis to %.3f %s/s and %.3f %s/s\xc2\xb2",
						   servoActuatorUnitsToAxisUnits(servoActuator->getVelocityLimit()),
						   Unit::getAbbreviatedString(positionUnit),
						   servoActuatorUnitsToAxisUnits(servoActuator->getAccelerationLimit()),
							Unit::getAbbreviatedString(positionUnit));
		ImGui::TextWrapped("Min Axis velocity is %.3f %s/s",
						   servoActuatorUnitsToAxisUnits(servoActuator->getMinVelocity()),
						   Unit::getAbbreviatedString(positionUnit));
		ImGui::PopStyleColor();
	}

	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", velocityLimit, Unit::getAbbreviatedString(positionUnit));
	ImGui::InputDouble("##VelLimit", &velocityLimit, 0.0, 0.0, velLimitString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	static char accLimitString[16];
	sprintf(accLimitString, "%.3f %s/s\xc2\xb2", accelerationLimit, Unit::getAbbreviatedString(positionUnit));
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &accelerationLimit, 0.0, 0.0, accLimitString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();

	ImGui::Text("Manual Acceleration");
	static char manualAccelerationString[256];
	sprintf(manualAccelerationString, "%.3f %s/s\xc2\xb2", manualAcceleration, Unit::getAbbreviatedString(positionUnit));
	ImGui::InputDouble("##manacc", &manualAcceleration, 0.0, 0.0, manualAccelerationString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	double halfWidgetWidth = (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0;

	ImGui::Separator();

	//----------------- REFERENCE SIGNALS, ORIGN AND HOMING -----------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Reference Signals & Homing");
	ImGui::PopFont();

	static auto showPositionReferenceDescription = [](PositionReferenceSignal type) {
		switch (type) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			ImGui::TextWrapped("Single Limit Signal at the negative end of the machine travel."
				"\nHoming will move the machine in the negative direction"
				"\nAxis Origin is set at the low limit.");
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			ImGui::TextWrapped("Two Limit Signals at each end of the axis travel."
				"\nHoming will first move the axis in the specified direction, then in the other direction"
				"\nAxis Origin is set at the low limit.");
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			ImGui::TextWrapped("Single Reference Signal inside the axis travel range."
				"\nHoming will find the signal using the specified direction."
				"\nAxis Origin is set at the reference signal.");
			break;
		case PositionReferenceSignal::NO_SIGNAL:
			ImGui::TextWrapped("No reference Signal, the machine is positionned using only feedback data."
				"\nAxis origin is set by manually moving the machine to the desired position and capturing the origin.");
			break;
		}
	};

	ImGui::Text("Reference Signal Type");
	if (ImGui::BeginCombo("##MovementType", Enumerator::getDisplayString(positionReferenceSignal))) {
		
		for (auto& type : Enumerator::getTypes<PositionReferenceSignal>()) {
			
			if(positionUnitType == Unit::DistanceType::LINEAR && !isLinearPositionReferenceSignal(type.enumerator)) continue;
			else if(positionUnitType == Unit::DistanceType::ANGULAR && !isAngularPositionReferenceSignal(type.enumerator)) continue;
			
			bool selected = positionReferenceSignal == type.enumerator;
			if (ImGui::Selectable(type.displayString, selected)) {
				setPositionReferenceSignalType(type.enumerator);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetNextWindowSize(glm::vec2(ImGui::GetTextLineHeight() * 20.0, 0));
				ImGui::BeginTooltip();
				showPositionReferenceDescription(type.enumerator);
				ImGui::EndTooltip();
			}
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
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			if (!lowLimitSignalPin->isConnected()) ImGui::TextWrapped("No Negative Limit Signal Connected.");
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			if (!lowLimitSignalPin->isConnected()) ImGui::TextWrapped("No Negative Limit Signal Connected.");
			if (!highLimitSignalPin->isConnected()) ImGui::TextWrapped("No Positive Limit Signal Connected.");
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			if (!referenceSignalPin->isConnected()) ImGui::TextWrapped("No Reference Signal Connected.");
			break;
			case PositionReferenceSignal::NO_SIGNAL:
				break;
		}
		ImGui::PopStyleColor();

		static char homVelString[16];
		sprintf(homVelString, "%.3f %s/s", homingVelocityCoarse, Unit::getAbbreviatedString(positionUnit));

		switch(positionReferenceSignal){
			case PositionReferenceSignal::NO_SIGNAL:
				break;
			default:
				ImGui::Text("Homing Velocity (Coarse)");
				ImGui::InputDouble("##gomvelcoar", &homingVelocityCoarse);
				if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
				
				ImGui::Text("Homing Velocity (Fine)");
				ImGui::InputDouble("##homvelfin", &homingVelocityFine);
				if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
				
				break;
		}
		
		switch (positionReferenceSignal) {
			case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
				ImGui::Text("Homing Direction");
				if (ImGui::BeginCombo("##HomingDirection", Enumerator::getDisplayString(homingDirection))) {
					for (auto& type : Enumerator::getTypes<HomingDirection>()) {
						bool selected = homingDirection == type.enumerator;
						if (ImGui::Selectable(type.displayString, selected)) homingDirection = type.enumerator;
					}
					ImGui::EndCombo();
				}
				break;
			default:
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
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			ImGui::TextWrapped("Axis is limited between Lower Limit Signal (Origin) and an Upper Limit Parameter.");
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			ImGui::TextWrapped("Axis is limited between Lower and Upper Limit Signals.");
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			ImGui::TextWrapped("Axis is limited between Lower and Upper Limit Parameters.");
			break;
		case PositionReferenceSignal::NO_SIGNAL:
			ImGui::TextWrapped("Axis is limited between Lower and Upper Limit Parameters.");
			break;
	}
	ImGui::Text("Movement range is from %.3f %s to %.3f %s",
				getLowPositionLimit(),
				Unit::getDisplayStringPlural(positionUnit),
				getHighPositionLimit(),
				Unit::getDisplayStringPlural(positionUnit));
	ImGui::PopStyleColor();

	const char * machineUnitShortString = Unit::getAbbreviatedString(positionUnit);
	static char negDevString[16];
	sprintf(negDevString, "%.3f %s", lowPositionLimit, machineUnitShortString);
	static char posDevString[16];
	sprintf(posDevString, "%.3f %s", highPositionLimit, machineUnitShortString);

	auto lowLimitSetting = [&](bool disableField){
		ImGui::Text("Max Negative Distance From Origin");
		ImGui::SetNextItemWidth(halfWidgetWidth);
		ImGui::BeginDisabled(disableField);
		ImGui::InputDouble("##MaxNegativeDeviation", &lowPositionLimit, 0.0, 0.0, negDevString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		ImGui::EndDisabled();
		if (lowPositionLimit > 0.0) lowPositionLimit = 0.0;
		ImGui::SameLine();
		ImGui::Checkbox("Enable Lower Axis Limit", &b_enableLowLimit);
	};
	
	auto highLimitSetting = [&](bool disableField){
		ImGui::Text("Max Positive Distance From Origin");
		ImGui::SetNextItemWidth(halfWidgetWidth);
		ImGui::BeginDisabled(disableField);
		ImGui::InputDouble("##MaxDeviation", &highPositionLimit, 0.0, 0.0, posDevString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		ImGui::EndDisabled();
		if (highPositionLimit < 0.0) highPositionLimit = 0.0;
		ImGui::SameLine();
		ImGui::Checkbox("Enable Upper Axis Limit", &b_enableHighLimit);
	};
	
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			lowLimitSetting(true);
			highLimitSetting(false);
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			lowLimitSetting(true);
			highLimitSetting(true);
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
		case PositionReferenceSignal::NO_SIGNAL:
			lowLimitSetting(false);
			highLimitSetting(false);
			break;
	}

	ImGui::Checkbox("Also Limit to Position Feedback Working Range", &limitToFeedbackWorkingRange);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	static char lowClearanceString[16];
	sprintf(lowClearanceString, "%.3f %s", lowLimitClearance, Unit::getAbbreviatedString(positionUnit));
	ImGui::Text("Low Limit Clearance");
	ImGui::InputDouble("##lowlimitclearance", &lowLimitClearance, 0.0, 0.0, lowClearanceString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	static char highClearanceString[16];
	sprintf(highClearanceString, "%.3f %s", highLimitClearance, Unit::getAbbreviatedString(positionUnit));
	ImGui::Text("Low Limit Clearance");
	ImGui::InputDouble("##highlimitclearance", &highLimitClearance, 0.0, 0.0, highClearanceString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();

}








void PositionControlledAxis::devicesGui() {

	//======================== CONNECTED DEVICES ==========================

	glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 6, ImGui::GetTextLineHeight() * 1.5);

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
	float maxV = velocityLimit;
	float minV = -velocityLimit;
	float minA = -accelerationLimit;
	float maxA = accelerationLimit;

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
