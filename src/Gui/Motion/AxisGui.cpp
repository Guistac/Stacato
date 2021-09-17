#include <pch.h>

#include "Motion/Axis/Axis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Gui/Framework/Fonts.h"

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

		float widgetWidth = ImGui::GetContentRegionAvail().x;

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Velocity Control");
		ImGui::PopFont();

		float targetVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::SliderFloat("##Velocity", &targetVelocity_degreesPerSecond, -velocityLimit_degreesPerSecond, velocityLimit_degreesPerSecond, "%.3f deg/s");
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::InputDouble("##ManAcceleration", &defaultManualAcceleration_degreesPerSecondSquared, 0.0, 0.0, "%.3f deg/s2");
		float velocityProgress = (profileVelocity_degreesPerSecond + velocityLimit_degreesPerSecond) / (2 * velocityLimit_degreesPerSecond);
		ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeight()));
		if (ImGui::Button("Stop##Velocity", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) targetVelocity_degreesPerSecond = 0.0;

		ImGui::Separator();

		//------------------------- POSITION CONTROLS --------------------------

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Position Control");
		ImGui::PopFont();

		static float targetPosition = 0.0;
		static float targetVelocity = defaultManualVelocity_degreesPerSecond;
		static float targetAcceleration = defaultManualAcceleration_degreesPerSecondSquared;
		float tripleWidgetWidth = (widgetWidth - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
		ImGui::SetNextItemWidth(tripleWidgetWidth);
		ImGui::InputFloat("##TargetPosition", &targetPosition, 0.0, 0.0, "%.3f deg");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tripleWidgetWidth);
		ImGui::InputFloat("##TargetVelocity", &targetVelocity, 0.0, 0.0, "%.3f deg/s");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tripleWidgetWidth);
		ImGui::InputFloat("##TargetAcceleration", &targetAcceleration, 0.0, 0.0, "%.3f deg/s2");
		float doubleWidgetWidth = (widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0;
		glm::vec2 doubleButtonWidth((widgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 1.5);
		if (ImGui::Button("Move", doubleButtonWidth)) {}
		ImGui::SameLine();
		if (ImGui::Button("Fast Move", doubleButtonWidth)) {}
		if (ImGui::Button("Stop##Target", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) targetVelocity_degreesPerSecond = 0.0;

		velocityControlTarget_degreesPerSecond = targetVelocity_degreesPerSecond;

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
			if (ImGui::Button("Start Homing", doubleButtonWidth)) {}
			ImGui::SameLine();
			if (ImGui::Button("Cancel Homing", doubleButtonWidth)) {}
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

			std::shared_ptr<FeedbackDevice> feedbackDevice = link->getInputData()->getFeedbackDevice();

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

			if (feedbackDevice->hasError()) {
				if (ImGui::Button("Clear Error", buttonSize)) feedbackDevice->clearError();
				ImGui::SameLine();
				ImGui::TextWrapped(feedbackDevice->getErrorString());
			}

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

			if (gpioDevice->hasError()) {
				if (ImGui::Button("Clear Error", buttonSize)) gpioDevice->clearError();
				ImGui::SameLine();
				ImGui::TextWrapped(gpioDevice->getErrorString());
			}

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

			if (actuatorDevice->hasError()) {
				if (ImGui::Button("Clear Error", buttonSize)) actuatorDevice->clearError();
				ImGui::SameLine();
				ImGui::TextWrapped(actuatorDevice->getErrorString());
			}


			if (actuatorDevice->isEnabled()) { if (ImGui::Button("Disable", buttonSize)) actuatorDevice->disable(); }
			else if (ImGui::Button("Enable", buttonSize)) actuatorDevice->enable();

			ImGui::PopID();

		}

		ImGui::EndChild();
	}
}