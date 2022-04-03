#include <pch.h>

#include "Motion/Axis/VelocityControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/SubDevice.h"
#include "Environnement/DeviceNode.h"

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
	
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if(isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Enabled", largeDoubleButtonSize);
	}else if(isReady()){
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Ready", largeDoubleButtonSize);
	}else{
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Disabled", largeDoubleButtonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	
	ImGui::SameLine();
	if(isEnabled()){
		if(ImGui::Button("Disable", largeDoubleButtonSize)) disable();
	}else if(isReady()){
		if(ImGui::Button("Enable", largeDoubleButtonSize)) enable();
	}else{
		ImGui::BeginDisabled();
		ImGui::Button("Not Ready", largeDoubleButtonSize);
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
	
	if(ImGui::Button("Fast Stop", largeSingleButtonSize)) fastStop();
	
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
		if(actuatorParentDevice) ImGui::Text("%s on %s", actuator->getName(), actuator->parentDevice->getName());
		else ImGui::Text("%s on Node %s", actuator->getName(), actuatorPin->getConnectedPin()->getNode()->getName());

		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Position Unit:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", actuator->positionUnit->plural);

		ImGui::Text("Actuator %s/s per Axis %s/s :", actuator->positionUnit->abbreviated, positionUnit->abbreviated);
		ImGui::InputDouble("##actuatorCoupling", &actuatorUnitsPerAxisUnits);
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
		ImGui::TextWrapped("Max actuator velocity is %.1f %s/s and max acceleration is %.1f %s/s\xc2\xb2",
						   actuator->getVelocityLimit(),
						   actuatorUnitAbbreviated,
						   actuator->getAccelerationLimit(),
						   actuatorUnitAbbreviated);
		const char* axisUnitAbbreviated = positionUnit->abbreviated;
		ImGui::TextWrapped("Axis is limited to %.3f %s/s and %.3f %s/s\xc2\xb2",
						   actuatorUnitsToAxisUnits(actuator->getVelocityLimit()),
						   axisUnitAbbreviated,
						   actuatorUnitsToAxisUnits(actuator->getAccelerationLimit()),
						   axisUnitAbbreviated);
		ImGui::PopStyleColor();
	}

	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", velocityLimit, positionUnit->abbreviated);
	ImGui::InputDouble("##VelLimit", &velocityLimit, 0.0, 0.0, velLimitString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	static char accLimitString[16];
	sprintf(accLimitString, "%.3f %s/s\xc2\xb2", accelerationLimit, positionUnit->abbreviated);
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &accelerationLimit, 0.0, 0.0, accLimitString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
	
	static char manualAccelerationString[16];
	sprintf(manualAccelerationString, "%.3f %s/s\xc2\xb2", manualAcceleration, positionUnit->abbreviated);
	ImGui::Text("Manul Controls Acceleration");
	ImGui::InputDouble("##ManAccLimit", &manualAcceleration, 0.0, 0.0, manualAccelerationString);
	if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
}

void VelocityControlledAxis::metricsGui() {}
void VelocityControlledAxis::devicesGui() {}
