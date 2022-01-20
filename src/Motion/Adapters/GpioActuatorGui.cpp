#include <pch.h>

#include "GpioActuator.h"

#include <imgui.h>


#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "NodeGraph/Device.h"

void GpioActuator::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Controls")){
		controlGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Settings")){
		settingsGui();
		ImGui::EndTabItem();
	}
}

void GpioActuator::controlGui(){
	
}

void GpioActuator::settingsGui(){
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Devices");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Gpio Device: ");
	ImGui::PopFont();
	ImGui::SameLine();
	if(isGpioDeviceConnected()){
		std::shared_ptr<GpioDevice> gpioDevice = getGpioDevice();
		ImGui::Text("%s on %s", gpioDevice->getName(), gpioDevice->parentDevice->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	ImGui::Separator();
	
	if(isGpioDeviceConnected()){
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Actuator Limits");
		ImGui::PopFont();
		
		ImGui::Text("Position Unit");
		if(ImGui::BeginCombo("##actuatorUnit", Enumerator::getDisplayString(actuator->positionUnit))){
			for(auto& type : Enumerator::getTypes<PositionUnit>()){
				if(isLinearPositionUnit(type.enumerator)) continue;
				if(ImGui::Selectable(type.displayString, type.enumerator == actuator->positionUnit)){
					actuator->positionUnit = type.enumerator;
				}
			}
			ImGui::EndCombo();
		}
		
		ImGui::Text("Velocity Limit");
		static char servoVelocityLimitString[256];
		sprintf(servoVelocityLimitString, "%.3f %s/s", actuator->velocityLimit_positionUnitsPerSecond, Unit::getAbbreviatedString(actuator->positionUnit));
		ImGui::InputDouble("##velocityLimit", &actuator->velocityLimit_positionUnitsPerSecond, 0.0, 0.0, servoVelocityLimitString);
		
		ImGui::Text("Acceleration Limit");
		static char servoAccelerationLimitString[256];
		sprintf(servoAccelerationLimitString, "%.3f %s/s2", actuator->accelerationLimit_positionUnitsPerSecondSquared, Unit::getAbbreviatedString(actuator->positionUnit));
		ImGui::InputDouble("##accelerationLimit", &actuator->accelerationLimit_positionUnitsPerSecondSquared, 0.0, 0.0, servoAccelerationLimitString);

		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Control Signal");
		ImGui::PopFont();
		
		ImGui::Text("Analog Signal Range");
		ImGui::InputDouble("##outputVoltageRange", &controlSignalRange);
		
		ImGui::Checkbox("Control Signal is Centered on Zero", &b_controlSignalIsCenteredOnZero);
		
		ImGui::Text("Control Signal ranges from %.1f to %.1f and is centered on %.1f",
					getControlSignalLowLimit(),
					getControlSignalHighLimit(),
					getControlSignalZero());
		
		ImGui::Text("Control Signal Units Per Servo Actuator %s", Enumerator::getDisplayString(actuator->positionUnit));
		ImGui::InputDouble("##UnitConversion", &controlSignalUnitsPerActuatorVelocityUnit);
		
		ImGui::Checkbox("Invert Control Signal Range", &b_invertControlSignal);
		
		ImGui::Text("Control Signal Limits Velocity to %.1f %s/s",
					getControlSignalLimitVelocity(),
					Unit::getAbbreviatedString(actuator->positionUnit));
		
		ImGui::Text("At 0.0 %s/s, Control Signal is %.1f",
					Unit::getAbbreviatedString(actuator->positionUnit),
					actuatorVelocityToControlSignal(0.0));
		
		ImGui::Text("At %.1f %s/s, Control Signal is %.1f",
					actuator->velocityLimit_positionUnitsPerSecond,
					Unit::getAbbreviatedString(actuator->positionUnit),
					actuatorVelocityToControlSignal(actuator->velocityLimit_positionUnitsPerSecond));
		
		ImGui::Text("At %.1f Control Signal, Velocity is %.1f %s/s",
					getControlSignalZero(),
					controlSignalToActuatorVelocity(getControlSignalZero()),
					Unit::getAbbreviatedString(actuator->positionUnit));

		ImGui::Text("At %.1f Control Signal, Velocity is %.1f %s/s",
					getControlSignalHighLimit(),
					controlSignalToActuatorVelocity(getControlSignalHighLimit()),
					Unit::getAbbreviatedString(actuator->positionUnit));

		ImGui::Separator();
		
		
		
		
	}
	
	
}
