#include <pch.h>

#include "Motion/Axis/VelocityControlledAxis.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Motion/SubDevice.h"
#include "NodeGraph/Device.h"

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

void VelocityControlledAxis::settingsGui() {
	
	//------------------ GENERAL MACHINE SETTINGS -------------------------

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Axis Settings");
	ImGui::PopFont();

	ImGui::Text("Movement Type :");
	if (ImGui::BeginCombo("##AxisUnitType", Enumerator::getDisplayString(positionUnitType))) {
		for (auto& type : Enumerator::getTypes<PositionUnitType>()) {
			if (ImGui::Selectable(type.displayString, positionUnitType == type.enumerator)) {
				setPositionUnitType(type.enumerator);
			}
		}
		ImGui::EndCombo();
	}

	float widgetWidth = ImGui::GetItemRectSize().x;

	ImGui::Text("Position Unit :");
	if (ImGui::BeginCombo("##AxisUnit", Unit::getDisplayString(positionUnit))) {
		
		for(auto& type : Unit::getTypes<PositionUnit>()){
			switch(positionUnitType){
				case PositionUnitType::LINEAR:
					if(!isLinearPositionUnit(type.enumerator)) continue;
					break;
				case PositionUnitType::ANGULAR:
					if(!isAngularPositionUnit(type.enumerator)) continue;
					break;
			}
			if (ImGui::Selectable(type.displayString, positionUnit == type.enumerator)) setPositionUnit(type.enumerator);
		}
		
		ImGui::EndCombo();
	}

	//---------------------- ACTUATOR -------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Actuator");
	ImGui::PopFont();

	if (isActuatorDeviceConnected()) {
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		std::shared_ptr<Device> actuatorParentDevice = actuator->parentDevice;
		
		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Device:");
		ImGui::PopFont();
		ImGui::SameLine();
		if(actuatorParentDevice) ImGui::Text("%s on %s", actuator->getName(), actuator->parentDevice->getName());
		else ImGui::Text("%s on Node %s", actuator->getName(), actuatorPin->getConnectedPin()->getNode()->getName());

		ImGui::PushFont(Fonts::robotoBold15);
		ImGui::Text("Position Unit:");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text("%s", Unit::getDisplayStringPlural(actuator->positionUnit));

		ImGui::Text("%s %s per Machine %s :", actuator->getName(), Unit::getDisplayStringPlural(actuator->positionUnit), Unit::getDisplayString(positionUnit));
		ImGui::InputDouble("##actuatorCoupling", &actuatorUnitsPerAxisUnits);

		if (actuatorUnitsPerAxisUnits < 0.0) actuatorUnitsPerAxisUnits = 0.0;
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, glm::vec4(1.0, 0.0, 0.0, 1.0));
		ImGui::TextWrapped("No Actuator device connected.");
		ImGui::PopStyleColor();
	}
	
	//-------------------------- KINEMATIC LIMITS ----------------------------

	ImGui::Separator();

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Kinematic Limits");
	ImGui::PopFont();

	if (isActuatorDeviceConnected()) {
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		const char* actuatorUnitAbbreviated = Unit::getAbbreviatedString(actuator->getPositionUnit());
		ImGui::TextWrapped("Max actuator velocity is %.1f %s/s and max acceleration is %.1f %s/s\xc2\xb2",
						   actuator->getVelocityLimit(),
						   actuatorUnitAbbreviated,
						   actuator->getAccelerationLimit(),
						   actuatorUnitAbbreviated);
		const char* axisUnitAbbreviated = Unit::getAbbreviatedString(positionUnit);
		ImGui::TextWrapped("Machine is limited to %.3f %s/s and %.3f %s/s\xc2\xb2",
						   actuatorUnitsToAxisUnits(actuator->getVelocityLimit()),
						   axisUnitAbbreviated,
						   actuatorUnitsToAxisUnits(actuator->getAccelerationLimit()),
						   axisUnitAbbreviated);
		ImGui::PopStyleColor();
	}

	ImGui::Text("Velocity Limit");
	static char velLimitString[16];
	sprintf(velLimitString, "%.3f %s/s", velocityLimit_axisUnitsPerSecond, Unit::getAbbreviatedString(positionUnit));
	ImGui::InputDouble("##VelLimit", &velocityLimit_axisUnitsPerSecond, 0.0, 0.0, velLimitString);
	static char accLimitString[16];
	sprintf(accLimitString, "%.3f %s/s\xc2\xb2", accelerationLimit_axisUnitsPerSecondSquared, Unit::getAbbreviatedString(positionUnit));
	ImGui::Text("Acceleration Limit");
	ImGui::InputDouble("##AccLimit", &accelerationLimit_axisUnitsPerSecondSquared, 0.0, 0.0, accLimitString);

}

void VelocityControlledAxis::metricsGui() {}
void VelocityControlledAxis::devicesGui() {}
void VelocityControlledAxis::controlsGui() {}
