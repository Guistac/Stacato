#include <pch.h>

#include "Motion/Axis/VelocityControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"


#include "Motion/SubDevice.h"
#include "Environnement/NodeGraph/DeviceNode.h"


/*

void VelocityControlledAxis::nodeSpecificGui() {
	if (ImGui::BeginTabItem("Controls")) {
		if (ImGui::BeginChild("Controls")) {
			controlsGui();
			ImGui::EndChild();
		}
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




void VelocityControlledAxis::controlsGui() {
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	bool axisControlledExternally = isAxisPinConnected();
	
	if(axisControlledExternally){
		ImGui::TextWrapped("Axis is Controlled by Node '%s'."
						   "\nManual controls are disabled.",
						   velocityControlledAxisPin->getConnectedPin()->getNode()->getName());
	}
	ImGui::BeginDisabled(axisControlledExternally);
		
	float singleWidgetWidth = ImGui::GetContentRegionAvail().x;
	glm::vec2 progressBarSize(singleWidgetWidth, ImGui::GetFrameHeight());
	glm::vec2 largeDoubleButtonSize((singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);
	glm::vec2 largeSingleButtonSize(singleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);
	
	
	if(isEmergencyStopActive()){
		bool blink = fmod(Timing::getProgramTime_seconds(), 0.5) > 0.25;
		backgroundText("E-Stop", largeDoubleButtonSize, blink ? Colors::yellow : Colors::red);
	}else{
		switch(state){
			case DeviceState::OFFLINE:
				backgroundText("Offline", largeDoubleButtonSize, Colors::blue);
				break;
			case DeviceState::NOT_READY:
				backgroundText("Not Ready", largeDoubleButtonSize, Colors::red);
				break;
			case DeviceState::READY:
				backgroundText("Ready", largeDoubleButtonSize, Colors::yellow);
				break;
			case DeviceState::ENABLED:
				backgroundText("Enabled", largeDoubleButtonSize, Colors::green);
				break;
		}
	}
	
	ImGui::SameLine();
	if(isEnabled()){
		if(ImGui::Button("Disable", largeDoubleButtonSize)) disable();
	}else{
		ImGui::BeginDisabled(state != DeviceState::READY);
		if(ImGui::Button("Enable", largeDoubleButtonSize)) enable();
		ImGui::EndDisabled();
	}
	
	ImGui::EndDisabled();
		
	
	bool disableManualControls = axisControlledExternally || !isEnabled();
	ImGui::BeginDisabled(disableManualControls);
	
	ImGui::Text("Manual Velocity:");
	static char velocityCommandString[256];
	sprintf(velocityCommandString, "%.3f %s/s", manualVelocityDisplay, positionUnit->abbreviated);
	ImGui::SetNextItemWidth(singleWidgetWidth);
	ImGui::SliderFloat("##manVel", &manualVelocityDisplay, -getVelocityLimit(), getVelocityLimit(), velocityCommandString);
	if (ImGui::IsItemActive()) setVelocity(manualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		manualVelocityDisplay = 0.0;
		setVelocity(0.0);
	}
	
	float velocityProgress = std::abs(motionProfile.getVelocity() / getVelocityLimit());
	static char velocityString[256];
	sprintf(velocityString, "%.3f %s/s", motionProfile.getVelocity(), positionUnit->abbreviated);
	ImGui::ProgressBar(velocityProgress, progressBarSize, velocityString);
	
	float width = ImGui::GetItemRectSize().x;
	float quadWidgetWidth = (width - 3.0 * ImGui::GetStyle().ItemSpacing.x) / 4.0;
	ImVec2 quadFieldSize(quadWidgetWidth, ImGui::GetTextLineHeight() * 2.0);
	
	
	backgroundText("Low Limit", quadFieldSize, !*lowLimitSignal ? Colors::darkYellow : Colors::yellow);
	ImGui::SameLine();
	backgroundText("Low Slowdown", quadFieldSize, !*lowSlowdownSignal ? Colors::darkGreen : Colors::green);
	ImGui::SameLine();
	backgroundText("High Slowdown", quadFieldSize, !*highSlowdownSignal ? Colors::darkGreen : Colors::green);
	ImGui::SameLine();
	backgroundText("High Limit", quadFieldSize, !*highLimitSignal ? Colors::darkYellow : Colors::yellow);
	
	ImGui::EndDisabled();
}




void VelocityControlledAxis::settingsGui() {
	
	//------------------ GENERAL MACHINE SETTINGS -------------------------

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();

	ImGui::Text("Movement Type :");
	if (ImGui::BeginCombo("##AxisUnitType", Enumerator::getDisplayString(movementType))) {
		for (auto& type : Enumerator::getTypes<MovementType>()) {
			if (ImGui::Selectable(type.displayString, movementType == type.enumerator)) {
				setMovementType(type.enumerator);
			}
		}
		ImGui::EndCombo();
	}

	float widgetWidth = ImGui::GetItemRectSize().x;

	ImGui::Text("Position Unit :");
	if (ImGui::BeginCombo("##AxisUnit", positionUnit->singular)) {
		switch(movementType){
			case MovementType::LINEAR:
				for(Unit linearUnit : Units::LinearDistance::get()){
					if (ImGui::Selectable(linearUnit->singular, positionUnit == linearUnit)) setPositionUnit(linearUnit);
				}
				break;
			case MovementType::ROTARY:
				for(Unit angularUnit : Units::AngularDistance::get()){
					if (ImGui::Selectable(angularUnit->singular, positionUnit == angularUnit)) setPositionUnit(angularUnit);
				}
				break;
		}
		ImGui::EndCombo();
	}

	//---------------------- ACTUATOR -------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Actuator");
	ImGui::PopFont();

	if (isActuatorDeviceConnected()) {
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		std::shared_ptr<Device> actuatorParentDevice = actuator->parentDevice;
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Device:");
		ImGui::PopFont();
		ImGui::SameLine();
		if(actuatorParentDevice) ImGui::Text("%s on %s", actuator->getName().c_str(), actuator->parentDevice->getName());
		else ImGui::Text("%s on Node %s", actuator->getName().c_str(), actuatorPin->getConnectedPin()->getNode()->getName());

		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Position Unit:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", actuator->getPositionUnit()->plural);

		ImGui::Text("Actuator %s per Axis %s :", actuator->getPositionUnit()->plural, positionUnit->singular);
		actuatorUnitsPerAxisUnits->gui();
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("No Actuator device connected.");
		ImGui::PopStyleColor();
	}
	
	//-------------------------- KINEMATIC LIMITS ----------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Kinematic Limits");
	ImGui::PopFont();

	if (isActuatorDeviceConnected()) {
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		const char* actuatorUnitAbbreviated = actuator->getPositionUnit()->abbreviated;
		ImGui::TextWrapped("Max actuator velocity is %.1f%s/s", actuator->getVelocityLimit(), actuatorUnitAbbreviated);
		ImGui::TextWrapped("Max actuator acceleration is %.1f%s/s\xc2\xb2", actuator->getAccelerationLimit(), actuatorUnitAbbreviated);
		ImGui::TextWrapped("Max actuator deceleration is %.1f%s/s\xc2\xb2", actuator->getDecelerationLimit(), actuatorUnitAbbreviated);
		const char* axisUnitAbbreviated = getPositionUnit()->abbreviated;
		ImGui::TextWrapped("Axis velocity is limited to %.3f %s/s", actuatorUnitsToAxisUnits(actuator->getVelocityLimit()), axisUnitAbbreviated);
		ImGui::TextWrapped("Axis acceleration is limited to %.3f %s/s\xc2\xb2", actuatorUnitsToAxisUnits(actuator->getAccelerationLimit()), axisUnitAbbreviated);
		ImGui::TextWrapped("Axis deceleration is limited to %.3f %s/s\xc2\xb2", actuatorUnitsToAxisUnits(actuator->getDecelerationLimit()), axisUnitAbbreviated);
		ImGui::PopStyleColor();
	}

	ImGui::Text("Velocity Limit");
	velocityLimit->gui();
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Acceleration Limit");
	accelerationLimit->gui();
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Deceleration Limit");
	decelerationLimit->gui();
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Manual Controls Acceleration");
	manualAcceleration->gui();
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Manual Controls Deceleration");
	manualDeceleration->gui();
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	ImGui::Text("Slowdown Velocity");
	slowdownVelocity->gui();
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
}

void VelocityControlledAxis::metricsGui() {}
void VelocityControlledAxis::devicesGui() {}

*/
