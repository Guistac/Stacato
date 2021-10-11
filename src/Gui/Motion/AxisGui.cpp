#include <pch.h>

#include "Motion/Axis/Axis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"
#include <implot.h>

#include "NodeGraph/DeviceNode.h"


void Axis::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Controls")) {
		controlsGui();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {
		settingsGui();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {
		devicesGui();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Metrics")) {
		metricsGui();
		ImGui::EndTabItem();
	}
}




void Axis::controlsGui() {

	if (ImGui::BeginChild("ControlsGui")) {

		//====================== AXIS MANUAL CONTROLS ==============================

		static glm::vec4 redColor = glm::vec4(0.7, 0.1, 0.1, 1.0);
		static glm::vec4 greenColor = glm::vec4(0.3, 0.7, 0.1, 1.0);

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Axis Control");
		ImGui::PopFont();

		glm::vec2 buttonSize;
		int buttonCount = 2;
		buttonSize.x = (ImGui::GetContentRegionAvail().x - (buttonCount - 1) * ImGui::GetStyle().ItemSpacing.x) / buttonCount;
		buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button("Position Feedback", buttonSize);
		ImGui::SameLine();
		ImGui::Button("Position Reference", buttonSize);
		ImGui::PopItemFlag();


		if (areAllDevicesReady()) {
			ImGui::PushStyleColor(ImGuiCol_Button, greenColor);
			if (ImGui::Button("Disable Actuator", buttonSize)) disableAllActuators();
			ImGui::PopStyleColor();
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, redColor);
			if (ImGui::Button("Enable Actuator", buttonSize)) enableAllActuators();
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();

		if (isEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_Button, greenColor);
			if (ImGui::Button("Disable Axis", buttonSize)) disable();
			ImGui::PopStyleColor();
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, redColor);
			if (ImGui::Button("Enable Axis", buttonSize)) enable();
			ImGui::PopStyleColor();
		}


		//------------------- VELOCITY CONTROLS ------------------------

		ImGui::Text("Acceleration for manual controls");
		ImGui::InputDouble("##TargetAcceleration", &manualControlAcceleration_degreesPerSecond, 0.0, 0.0, "%.3f deg/s2");

		float widgetWidth = ImGui::GetContentRegionAvail().x;

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Velocity Control");
		ImGui::PopFont();

		ImGui::SetNextItemWidth(widgetWidth);
		if (ImGui::SliderFloat("##Velocity", &manualVelocityTarget_degreesPerSecond, -velocityLimit_degreesPerSecond, velocityLimit_degreesPerSecond, "%.3f deg/s")) {
			setVelocity(manualVelocityTarget_degreesPerSecond);
		}
		float velocityProgress = (profileVelocity_degreesPerSecond + velocityLimit_degreesPerSecond) / (2 * velocityLimit_degreesPerSecond);
		ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeight()));
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
		float targetProgress = targetProgress = MotionCurve::getMotionCurveProgress(currentProfilePointTime_seconds, targetCurveProfile);
		if (controlMode != ControlMode::POSITION_TARGET) targetProgress = 1.0;
		ImGui::ProgressBar(targetProgress);
		if (ImGui::Button("Stop##Target", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) {
			setVelocity(0.0);
		}

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


		ImGui::EndChild();
	}
}








void Axis::settingsGui() {

	if (ImGui::BeginChild("SettingsGui")) {

		//=================== AXIS SETTINGS ====================

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Axis Settings");
		ImGui::PopFont();

		//------------------ AXIS TYPE -------------------------

		ImGui::Text("Axis Type");
		if (ImGui::BeginCombo("##AxisType", getAxisType(axisUnitType)->displayName)) {
			for (AxisType& axisType : getAxisTypes()) {
				if (ImGui::Selectable(axisType.displayName, axisUnitType == axisType.unitType)) {
					axisUnitType = axisType.unitType;
					//if the axis type is changed but the axis unit is of the wrong type
					//change the axis unit to the first correct type automatically
					if (getPositionUnitType(axisPositionUnit)->type != axisType.unitType) {
						if (axisType.unitType == UnitType::ANGULAR) {
							axisPositionUnit = getAngularPositionUnits().front().unit;
						}
						else if (axisType.unitType == UnitType::LINEAR) {
							axisPositionUnit = getLinearPositionUnits().front().unit;
						}
					}
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Text("Axis Position Unit");
		if (ImGui::BeginCombo("##AxisUnit", getPositionUnitType(axisPositionUnit)->displayName)) {
			if (axisUnitType == UnitType::LINEAR) {
				for (PositionUnit& unit : getLinearPositionUnits()) {
					if (ImGui::Selectable(unit.displayName, axisPositionUnit == unit.unit)) axisPositionUnit = unit.unit;
				}
			}
			else if (axisUnitType == UnitType::ANGULAR) {
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

			ImGui::Text("Feedback Position Unit");
			if (ImGui::BeginCombo("##FeedbackUnit", getPositionUnitType(feedbackPositionUnit)->displayNamePlural)) {
				for (PositionUnit& unit : getAngularPositionUnits()) {
					if (ImGui::Selectable(unit.displayNamePlural, feedbackPositionUnit == unit.unit)) feedbackPositionUnit = unit.unit;
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Feedback %s per Axis %s", getPositionUnitType(feedbackPositionUnit)->displayNamePlural, getPositionUnitType(axisPositionUnit)->displayName);
			ImGui::InputDouble("##feedbackCoupling", &feedbackUnitsPerAxisUnits);
		}

		ImGui::Separator();

		//---------------------- ACTUATOR AND COMMAND -------------------------

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Actuator");
		ImGui::PopFont();

		ImGui::Text("Command Type");
		if (ImGui::BeginCombo("##CommandType", getCommandType(commandType)->displayName)) {
			for (CommandType& command : getCommandTypes()) {
				if (ImGui::Selectable(command.displayName, commandType == command.type)) commandType = command.type;
			}
			ImGui::EndCombo();
		}

		if (commandType == CommandType::Type::VELOCITY_COMMAND) {
			ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
			ImGui::TextWrapped("Actuators with velocity command are not yet supported. Actuators with velocity command would need a PID controller with position feedback to regulate the axis position");
			ImGui::PopStyleColor();
		}
		else {
			ImGui::Text("Command Position Unit");
			if (ImGui::BeginCombo("##CommandUnit", getPositionUnitType(commandPositionUnit)->displayName)) {
				for (PositionUnit& unit : getAngularPositionUnits()) {
					if (ImGui::Selectable(unit.displayName, commandPositionUnit == unit.unit)) commandPositionUnit = unit.unit;
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Actuator %s per Axis %s", getPositionUnitType(commandPositionUnit)->displayNamePlural, getPositionUnitType(axisPositionUnit)->displayName);
			ImGui::InputDouble("##actuatorCoupling", &commandUnitsPerAxisUnits);
		}

		ImGui::Separator();

		//-------------------------- KINEMATIC LIMITS ----------------------------

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Kinematic Limits");
		ImGui::PopFont();

		ImGui::Text("Velocity Limit (%s per second)", getAxisUnitStringPlural());
		ImGui::InputDouble("##VelLimit", &velocityLimit_degreesPerSecond, 0.0, 0.0, "%.3f u/s");
		ImGui::Text("Acceleration Limit (%s per second squared)", getAxisUnitStringPlural());
		ImGui::InputDouble("##AccLimit", &accelerationLimit_degreesPerSecondSquared, 0.0, 0.0, "%.3f u/s2");

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
			ImGui::TextWrapped("Single Limit Signal at the negative end of the axis travel. Homing will move the axis in the negative direction");
			break;
		case PositionReference::Type::HIGH_LIMIT:
			ImGui::Text("Max Deviation From High Limit");
			ImGui::InputDouble("##MaxDeviation", &allowedNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f deg");
			if (allowedNegativeDeviationFromReference_degrees > 0.0) allowedNegativeDeviationFromReference_degrees = 0.0;
			ImGui::Text("Homing Velocity");
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
			if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
			ImGui::TextWrapped("Single Limit Signal at the positive end of the axis travel. Homing will move the axis in the position direction");
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
			ImGui::TextWrapped("Two Limit Signals at each end of the axis travel. Homing will first move the axis in the specified direction, then the other direction");
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
			ImGui::TextWrapped("Single Limit Signal inside the axis travel range. Homing will find the position reference using the specified direction. The axis will not go over the max deviations from the position reference. (Not recommended for axis with physical limits)");
			break;
		case PositionReference::Type::NO_LIMIT:
			ImGui::Text("Max Positive Deviation");
			ImGui::InputDouble("##MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
			if (allowedPositiveDeviationFromReference_degrees < 0.0) allowedPositiveDeviationFromReference_degrees = 0.0;
			ImGui::Text("Max Negative Deviation");
			ImGui::InputDouble("##MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
			if (allowedNegativeDeviationFromReference_degrees > 0.0) allowedNegativeDeviationFromReference_degrees = 0.0;
			ImGui::TextWrapped("No Limit Signal. Setting of the origin has to be done by manually moving the axis to the desired position reference and resetting the position feedback. (Not recommended for position feedback types other than absolute)");
		}

		ImGui::EndChild();
	}
}











void Axis::devicesGui() {

	if (ImGui::BeginChild("DevicesGui")) {

		//======================== CONNECTED DEVICES ==========================

		glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 6, ImGui::GetTextLineHeight() * 1.5);

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback: ");
		ImGui::PopFont();

		for (auto link : feedbackDeviceLink->getLinks()) {

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

		ImGui::EndChild();
	}
}

void Axis::metricsGui() {
	if (ImGui::BeginChild("Metrics")) {

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

		if (positionPointCount) {
			ImPlot::SetNextPlotLimitsX(positionBuffer[0].x, positionBuffer[positionPointCount - 1].x, ImGuiCond_Always);
			ImPlot::FitNextPlotAxes(false, true, false, false);
		}
		ImPlot::SetNextPlotLimitsY(-velocityLimit_degreesPerSecond * 1.1, velocityLimit_degreesPerSecond * 1.1, ImGuiCond_Always, ImPlotYAxis_2);
		ImPlot::SetNextPlotLimitsY(-1.0, 1.0, ImGuiCond_Always, ImPlotYAxis_3);

		float zero = 0.0;
		float maxV = velocityLimit_degreesPerSecond;
		float minV = -velocityLimit_degreesPerSecond;

		ImPlotFlags plotFlags = ImPlotFlags_AntiAliased | ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild | ImPlotFlags_YAxis2 | ImPlotFlags_YAxis3;

		if(ImPlot::BeginPlot("##Metrics", 0, 0, glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 20.0), plotFlags)) {
		
			ImPlot::SetPlotYAxis(ImPlotYAxis_2);
			ImPlot::SetNextLineStyle(glm::vec4(0.3, 0.3, 0.3, 1.0), 2.0);
			ImPlot::PlotHLines("zero", &zero, 1);
			ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
			ImPlot::PlotHLines("MaxV", &maxV, 1);
			ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 0.5, 1.0), 2.0);
			ImPlot::PlotHLines("MinV", &minV, 1);
			ImPlot::SetNextLineStyle(glm::vec4(0.0, 0.0, 1.0, 1.0), 4.0);
			if (velocityPointCount > 0) ImPlot::PlotLine("Profile Velocity", &velocityBuffer[1].x, &velocityBuffer[1].y, velocityPointCount-1, 0, sizeof(glm::vec2));
			ImPlot::SetNextLineStyle(glm::vec4(0.5, 0.5, 0.5, 1.0), 4.0);
			if (accelerationPointCount > 0) ImPlot::PlotLine("Profile Acceleration", &accelerationBuffer[1].x, &accelerationBuffer[1].y, accelerationPointCount-1, 0, sizeof(glm::vec2));

			ImPlot::SetPlotYAxis(ImPlotYAxis_3);
			//ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 0.0, 1.0), 2.0);
			//if (positionErrorPointCount > 0) ImPlot::PlotLine("Position Error", &positionErrorBuffer[1].x, &positionErrorBuffer[1].y, positionErrorPointCount - 1, 0, sizeof(glm::vec2));
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 1.0, 1.0), 2.0);
			if (loadPointCount > 0) ImPlot::PlotLine("Load", &loadBuffer[1].x, &loadBuffer[1].y, loadPointCount - 1, 0, sizeof(glm::vec2));

			ImPlot::SetPlotYAxis(ImPlotYAxis_1);
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 1.0, 1.0, 1.0), 4.0);
			if (positionPointCount > 0) ImPlot::PlotLine("Profile Position", &positionBuffer[1].x, &positionBuffer[1].y, positionPointCount - 1, 0, sizeof(glm::vec2));
			ImPlot::SetNextLineStyle(glm::vec4(1.0, 0.0, 0.0, 1.0), 2.0);
			if (actualPositionPointCount > 0) ImPlot::PlotLine("Actual Position", &actualPositionBuffer[1].x, &actualPositionBuffer[1].y, actualPositionPointCount - 1, 0, sizeof(glm::vec2));

			ImPlot::EndPlot();
		}

		ImGui::EndChild();
	}
}