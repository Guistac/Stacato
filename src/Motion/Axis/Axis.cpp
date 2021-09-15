#include <pch.h>

#include "Axis.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <tinyxml2.h>

#include "Gui/Framework/Fonts.h"



#include "NodeGraph/DeviceNode.h"

std::vector<PositionFeedback> positionFeedbackTypes = {
	{PositionFeedbackType::ABSOLUTE_FEEDBACK, "Absolute Feedback", "Absolute"},
	{PositionFeedbackType::INCREMENTAL_FEEDBACK, "Incremental Feedback", "Incremental"},
	{PositionFeedbackType::NO_FEEDBACK, "No Feedback", "None"}
};

std::vector<PositionReference> positionReferenceTypes = {
	{PositionReferenceType::LOW_LIMIT, "Low Limit", "Low"},
	{PositionReferenceType::HIGH_LIMIT, "High Limit", "High"},
	{PositionReferenceType::LOW_AND_HIGH_LIMIT, "Low and High Limit", "LowHigh"},
	{PositionReferenceType::POSITION_REFERENCE, "Position Reference", "Reference"},
	{PositionReferenceType::NO_LIMIT, "No Limit", "None"}
};

std::vector<HomingDirection> homingDirectionTypes = {
	{HomingDirectionType::NEGATIVE, "Negative", "Negative"},
	{HomingDirectionType::POSITIVE, "Positive", "Positive"},
	{HomingDirectionType::DONT_CARE, "Don't Care", "DontCare"}
};


void Axis::process() {
	double updateTime_seconds = Timing::getTime_seconds();
	double deltaT_seconds = lastUpdateTime_seconds - updateTime_seconds;

	double deltaV_degreesPerSecond;
	deltaV_degreesPerSecond = defaultMovementAcceleration_degreesPerSecondSquared * deltaT_seconds;
	if (velocityControlTarget_degreesPerSecond > currentVelocity_degreesPerSecond) {
		if (currentVelocity_degreesPerSecond + deltaV_degreesPerSecond > velocityControlTarget_degreesPerSecond) currentVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
		else currentVelocity_degreesPerSecond += deltaV_degreesPerSecond;
	}
	else {
		deltaV_degreesPerSecond = -defaultMovementAcceleration_degreesPerSecondSquared * deltaT_seconds;
		if (currentVelocity_degreesPerSecond - deltaV_degreesPerSecond < velocityControlTarget_degreesPerSecond) currentVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
		else currentVelocity_degreesPerSecond -= deltaV_degreesPerSecond;
	}

	double deltaP_degrees = currentVelocity_degreesPerSecond * deltaT_seconds;

	double actualPosition = 0.0;
	if(positionFeedback->isConnected()) actualPosition = positionFeedback->getLinks().front()->getInputData()->getReal();
	positionCommand->set(actualPosition + deltaP_degrees / 360.0);


	lastUpdateTime_seconds = updateTime_seconds;
}

void Axis::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Controls")) {

		float widgetWidth = ImGui::GetContentRegionAvail().x;

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Velocity Control");
		ImGui::PopFont();

		float targetVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::SliderFloat("##Velocity", &targetVelocity_degreesPerSecond, -velocityLimit_degreesPerSecond, velocityLimit_degreesPerSecond, "%.3f deg/s");
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::InputDouble("##ManAcceleration", &defaultMovementAcceleration_degreesPerSecondSquared, 0.0, 0.0,"%.3f deg/s2");
		float velocityProgress = (currentVelocity_degreesPerSecond + velocityLimit_degreesPerSecond) / (2 * velocityLimit_degreesPerSecond);
		ImGui::ProgressBar(velocityProgress, ImVec2(widgetWidth, ImGui::GetTextLineHeight()));
		if (ImGui::Button("Stop##Velocity", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) targetVelocity_degreesPerSecond = 0.0;

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Manual Position Control");
		ImGui::PopFont();

		static float targetPosition = 0.0;
		static float targetVelocity = defaultMovementVelocity_degreesPerSecond;
		static float targetAcceleration = defaultMovementAcceleration_degreesPerSecondSquared;
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
		if(ImGui::Button("Stop##Target", glm::vec2(widgetWidth, ImGui::GetTextLineHeight() * 2))) targetVelocity_degreesPerSecond = 0.0;

		velocityControlTarget_degreesPerSecond = targetVelocity_degreesPerSecond;

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Homing");
		ImGui::PopFont();

		switch (positionReferenceType) {
		case PositionReferenceType::LOW_LIMIT:
		case PositionReferenceType::HIGH_LIMIT:
		case PositionReferenceType::LOW_AND_HIGH_LIMIT:
		case PositionReferenceType::POSITION_REFERENCE:
			if (ImGui::Button("Start Homing", doubleButtonWidth)) {}
			ImGui::SameLine();
			if (ImGui::Button("Cancel Homing", doubleButtonWidth)) {}
			break;
		case PositionReferenceType::NO_LIMIT:
			if (ImGui::Button("Reset Position Feedback")) {}
			break;
		}
		

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Settings")) {

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Ramp Limits");
		ImGui::PopFont();

		ImGui::Text("Velocity Limit (degrees per second)");
		ImGui::InputDouble("##VelLimit", &velocityLimit_degreesPerSecond, 0.0, 0.0, "%.3f d/s");
		ImGui::Text("Acceleration Limit (degrees per second squared)");
		ImGui::InputDouble("##AccLimit", &accelerationLimit_degreesPerSecondSquared, 0.0, 0.0, "%.3f d/s2");

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Limits");
		ImGui::PopFont();


		ImGui::Text("Limit Type");
		if (ImGui::BeginCombo("##PositionReference", positionReferenceTypes[positionReferenceType].displayName)) {
			for (int i = 0; i < positionReferenceTypes.size(); i++) {
				bool selected = positionReferenceType == positionReferenceTypes[i].type;
				if (ImGui::Selectable(positionReferenceTypes[i].displayName, selected)) positionReferenceType = positionReferenceTypes[i].type;
			}
			ImGui::EndCombo();
		}

		switch (positionReferenceType) {
		case PositionReferenceType::LOW_LIMIT:
			ImGui::Text("Max Deviation From Low Limit");
			ImGui::InputDouble("##MaxDeviation", &maxPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f deg");
			if (maxPositiveDeviationFromReference_degrees < 0.0) maxPositiveDeviationFromReference_degrees = 0.0;
			ImGui::Text("Homing Velocity");
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
			if(homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
			ImGui::TextWrapped("Single Limit Signal at the negative end of the axis travel. Homing will move the axis in the negative direction");
			break;
		case PositionReferenceType::HIGH_LIMIT:
			ImGui::Text("Max Deviation From High Limit");
			ImGui::InputDouble("##MaxDeviation", &maxNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f deg");
			if (maxNegativeDeviationFromReference_degrees > 0.0) maxNegativeDeviationFromReference_degrees = 0.0;
			ImGui::Text("Homing Velocity");
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
			if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
			ImGui::TextWrapped("Single Limit Signal at the positive end of the axis travel. Homing will move the axis in the position direction");
			break;
		case PositionReferenceType::LOW_AND_HIGH_LIMIT:
			ImGui::Text("Homing Direction");
			if (ImGui::BeginCombo("##HomingDirection", homingDirectionTypes[homingDirectionType].displayName)) {
				for (int i = 0; i < homingDirectionTypes.size(); i++) {
					bool selected = homingDirectionType == homingDirectionTypes[i].type;
					if (ImGui::Selectable(homingDirectionTypes[i].displayName, selected)) homingDirectionType = homingDirectionTypes[i].type;
				}
				ImGui::EndCombo();
			}
			ImGui::Text("Homing Velocity");
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
			if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
			ImGui::TextWrapped("Two Limit Signals at each end of the axis travel. Homing will first move the axis in the specified direction, then the other direction");
			break;
		case PositionReferenceType::POSITION_REFERENCE:
			ImGui::Text("Homing Direction");
			if (ImGui::BeginCombo("##HomingDirection", homingDirectionTypes[homingDirectionType].displayName)) {
				for (int i = 0; i < homingDirectionTypes.size(); i++) {
					bool selected = homingDirectionType == homingDirectionTypes[i].type;
					if (ImGui::Selectable(homingDirectionTypes[i].displayName, selected)) homingDirectionType = homingDirectionTypes[i].type;
				}
				ImGui::EndCombo();
			}
			ImGui::Text("Max Positive Deviation");
			ImGui::InputDouble("##MaxPositiveDeviation", &maxPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
			if (maxPositiveDeviationFromReference_degrees < 0.0) maxPositiveDeviationFromReference_degrees = 0.0;
			ImGui::Text("Max Negative Deviation");
			ImGui::InputDouble("##MaxNegativeDeviation", &maxNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
			if (maxNegativeDeviationFromReference_degrees > 0.0) maxNegativeDeviationFromReference_degrees = 0.0;
			ImGui::Text("Homing Velocity");
			ImGui::InputDouble("##HomingVelocity", &homingVelocity_degreesPerSecond, 0.0, 0.0, "%.3f deg/s");
			if (homingVelocity_degreesPerSecond < 0) homingVelocity_degreesPerSecond = abs(homingVelocity_degreesPerSecond);
			ImGui::TextWrapped("Single Limit Signal in the axis travel range. Homing will find the position reference using the specified direction. The axis will not go over the max deviations from the position reference. (Not recommended for axis with physical limits)");
			break;
		case PositionReferenceType::NO_LIMIT:
			ImGui::Text("Max Positive Deviation");
			ImGui::InputDouble("##MaxPositiveDeviation", &maxPositiveDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
			if (maxPositiveDeviationFromReference_degrees < 0.0) maxPositiveDeviationFromReference_degrees = 0.0;
			ImGui::Text("Max Negative Deviation");
			ImGui::InputDouble("##MaxNegativeDeviation", &maxNegativeDeviationFromReference_degrees, 0.0, 0.0, "%.3f");
			if (maxNegativeDeviationFromReference_degrees > 0.0) maxNegativeDeviationFromReference_degrees = 0.0;
			ImGui::TextWrapped("No Limit Signal. Setting of the origin has to be done by manually moving the axis to the desired position reference and resetting the position feedback. (Not recommended for position feedback types other than absolute)");
		}

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Devices")) {

		glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 6, ImGui::GetTextLineHeight() * 1.5);

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback: ");
		ImGui::PopFont();

		ImGui::Separator();

		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Actuators: ");
		ImGui::PopFont();

		for (auto link : deviceLink->getLinks()) {
			auto node = link->getInputData()->getNode();
			if (node->getType() == IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(node);

				ImGui::PushID(device->getName());

				ImGui::PushFont(Fonts::robotoBold15);
				ImGui::Text(device->getName());
				ImGui::PopFont();

				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

				ImGui::PushStyleColor(ImGuiCol_Button, device->isOnline() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
				ImGui::Button(device->isOnline() ? "Online" : "Offline", buttonSize);
				ImGui::PopStyleColor();

				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Button, device->isReady() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
				ImGui::Button(device->isReady() ? "Ready": "Not Ready", buttonSize);
				ImGui::PopStyleColor();

				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Button, device->isEnabled() ? glm::vec4(0.3, 0.7, 0.1, 1.0) : glm::vec4(0.7, 0.1, 0.1, 1.0));
				ImGui::Button(device->isEnabled() ? "Enabled" : "Disabled", buttonSize);
				ImGui::PopStyleColor();


				ImGui::PopItemFlag();

				if (device->hasError()) {
					if (ImGui::Button("Clear Error", buttonSize)) device->clearError();
					ImGui::SameLine();
					ImGui::TextWrapped(device->getErrorString());
				}
				

				if (device->isEnabled()) { if (ImGui::Button("Disable", buttonSize)) device->disable(); }
				else if (ImGui::Button("Enable", buttonSize)) device->enable();

				ImGui::PopID();

				ImGui::Separator();
			}
		}

		ImGui::EndTabItem();
	}
}


bool Axis::load(tinyxml2::XMLElement* xml) { 
	using namespace tinyxml2;

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not find Kinematic Kimits Attribute");
	double vel, acc;
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_degreesPerSecond", &vel) != XML_SUCCESS) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_degreesPerSecondSquared", &acc) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");
	XMLElement* defaultMovementParametersXML = xml->FirstChildElement("DefaultMovementParameters");
	if (!defaultMovementParametersXML) return Logger::warn("Could not load default movement parameters");
	double defVel, defAcc;
	if (defaultMovementParametersXML->QueryDoubleAttribute("defaultVelocity_degreesPerSecond", &defVel) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultMovementParametersXML->QueryDoubleAttribute("defaultAcceleration_degreesPerSecondSquared", &defAcc) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Position Reference Attributes");
	const char* referenceType;
	bool identifiedLimitType = false;
	PositionReferenceType refType;
	if (positionReferenceXML->QueryStringAttribute("ReferenceType", &referenceType) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	for (PositionReference& ref : positionReferenceTypes) {
		if (strcmp(referenceType, ref.saveName) == 0) {
			refType = ref.type;
			identifiedLimitType = true;
		}
	}
	if (!identifiedLimitType) return Logger::warn("Could not identify Reference Type");
	double maxPDeviation;
	double maxNDeviation;
	double homingVel;
	HomingDirectionType homingDir;
	bool homingDirIdentified = false;

	switch (refType) {
	case PositionReferenceType::LOW_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &maxPDeviation) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVel) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReferenceType::HIGH_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &maxNDeviation) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVel) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReferenceType::LOW_AND_HIGH_LIMIT:
	case PositionReferenceType::POSITION_REFERENCE:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &maxPDeviation) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &maxNDeviation) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
				if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVel) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		const char* homingDirString;
		if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirString) != XML_SUCCESS) return Logger::warn("Could not load homing direction");
		for (HomingDirection& dir : homingDirectionTypes) {
			if (strcmp(dir.saveName, homingDirString) == 0) {
				homingDir = dir.type;
				homingDirIdentified = true;
			}
		}
		if (!homingDirIdentified) return Logger::warn("Could not identify homing direction");
		break;
	case PositionReferenceType::NO_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &maxPDeviation) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &maxNDeviation) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
	}

	velocityLimit_degreesPerSecond = vel;
	accelerationLimit_degreesPerSecondSquared = acc;
	defaultMovementVelocity_degreesPerSecond = defVel;
	defaultMovementAcceleration_degreesPerSecondSquared = defAcc;
	positionReferenceType = refType;
	maxPositiveDeviationFromReference_degrees = maxPDeviation;
	maxNegativeDeviationFromReference_degrees = maxNDeviation;
	homingVelocity_degreesPerSecond = homingVel;
	homingDirectionType = homingDir;

	return true;
}

bool Axis::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_degreesPerSecond", velocityLimit_degreesPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_degreesPerSecondSquared", accelerationLimit_degreesPerSecondSquared);
	XMLElement* defaultMovementParametersXML = xml->InsertNewChildElement("DefaultMovementParameters");
	defaultMovementParametersXML->SetAttribute("defaultVelocity_degreesPerSecond", defaultMovementVelocity_degreesPerSecond);
	defaultMovementParametersXML->SetAttribute("defaultAcceleration_degreesPerSecondSquared", defaultMovementAcceleration_degreesPerSecondSquared);
	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReference");
	positionReferenceXML->SetAttribute("ReferenceType", positionReferenceTypes[positionReferenceType].saveName);
	switch (positionReferenceType) {
	case PositionReferenceType::LOW_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		break;
	case PositionReferenceType::HIGH_LIMIT:
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		break;
	case PositionReferenceType::LOW_AND_HIGH_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", homingDirectionTypes[homingDirectionType].saveName);
	case PositionReferenceType::POSITION_REFERENCE:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", homingDirectionTypes[homingDirectionType].saveName);
	case PositionReferenceType::NO_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
	}

	return false;
}