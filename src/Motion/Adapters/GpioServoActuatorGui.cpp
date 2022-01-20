#include <pch.h>

#include "GpioServoActuator.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "NodeGraph/Device.h"

void GpioServoActuator::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Controls")){
		controlGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Settings")){
		settingsGui();
		ImGui::EndTabItem();
	}
}

void GpioServoActuator::controlGui(){
	
}

void GpioServoActuator::settingsGui(){
	
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
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position Feedback Device: ");
	ImGui::PopFont();
	ImGui::SameLine();
	if(isPositionFeedbackConnected()){
		std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getPositionFeedbackDevice();
		ImGui::Text("%s on %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	ImGui::Separator();
	
	if(isPositionFeedbackConnected() && isGpioDeviceConnected()){
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Actuator Limits");
		ImGui::PopFont();
		
		ImGui::Text("Position Unit");
		if(ImGui::BeginCombo("##actuatorUnit", Enumerator::getDisplayString(servoActuator->positionUnit))){
			for(auto& type : Enumerator::getTypes<PositionUnit>()){
				if(isLinearPositionUnit(type.enumerator)) continue;
				if(ImGui::Selectable(type.displayString, type.enumerator == servoActuator->positionUnit)){
					servoActuator->positionUnit = type.enumerator;
				}
			}
			ImGui::EndCombo();
		}
		
		ImGui::Text("Velocity Limit");
		static char servoVelocityLimitString[256];
		sprintf(servoVelocityLimitString, "%.3f %s/s", servoActuator->velocityLimit_positionUnitsPerSecond, Unit::getAbbreviatedString(servoActuator->positionUnit));
		ImGui::InputDouble("##velocityLimit", &servoActuator->velocityLimit_positionUnitsPerSecond, 0.0, 0.0, servoVelocityLimitString);
		
		ImGui::Text("Acceleration Limit");
		static char servoAccelerationLimitString[256];
		sprintf(servoAccelerationLimitString, "%.3f %s/s2", servoActuator->accelerationLimit_positionUnitsPerSecondSquared, Unit::getAbbreviatedString(servoActuator->positionUnit));
		ImGui::InputDouble("##accelerationLimit", &servoActuator->accelerationLimit_positionUnitsPerSecondSquared, 0.0, 0.0, servoAccelerationLimitString);

		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Control Signal");
		ImGui::PopFont();
		
		auto feedbackDevice = getPositionFeedbackDevice();
		PositionUnit feedbackUnit = feedbackDevice->getPositionUnit();
		
		ImGui::Text("Analog Signal Range");
		ImGui::InputDouble("##outputVoltageRange", &controlSignalRange);
		
		ImGui::Checkbox("Control Signal is Centered on Zero", &b_controlSignalIsCenteredOnZero);
		
		ImGui::Text("Control Signal ranges from %.1f to %.1f and is centered on %.1f",
					getControlSignalLowLimit(),
					getControlSignalHighLimit(),
					getControlSignalZero());
		
		ImGui::Text("Control Signal Units Per Servo Actuator %s", Enumerator::getDisplayString(servoActuator->positionUnit));
		ImGui::InputDouble("##UnitConversion", &controlSignalUnitsPerActuatorVelocityUnit);
		
		ImGui::Checkbox("Invert Control Signal Range", &b_invertControlSignal);
		
		ImGui::Text("Control Signal Limits Velocity to %.1f %s/s",
					getControlSignalLimitVelocity(),
					Unit::getAbbreviatedString(servoActuator->positionUnit));
		
		ImGui::Text("At 0.0 %s/s, Control Signal is %.1f",
					Unit::getAbbreviatedString(servoActuator->positionUnit),
					actuatorVelocityToControlSignal(0.0));
		
		ImGui::Text("At %.1f %s/s, Control Signal is %.1f",
					servoActuator->velocityLimit_positionUnitsPerSecond,
					Unit::getAbbreviatedString(servoActuator->positionUnit),
					actuatorVelocityToControlSignal(servoActuator->velocityLimit_positionUnitsPerSecond));
		
		ImGui::Text("At %.1f Control Signal, Velocity is %.1f %s/s",
					getControlSignalZero(),
					controlSignalToActuatorVelocity(getControlSignalZero()),
					Unit::getAbbreviatedString(servoActuator->positionUnit));

		ImGui::Text("At %.1f Control Signal, Velocity is %.1f %s/s",
					getControlSignalHighLimit(),
					controlSignalToActuatorVelocity(getControlSignalHighLimit()),
					Unit::getAbbreviatedString(servoActuator->positionUnit));

		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback Scaling");
		ImGui::PopFont();
		
		ImGui::Text("Feedback %s per Actuator %s", Unit::getDisplayStringPlural(feedbackDevice->getPositionUnit()), Unit::getDisplayString(servoActuator->getPositionUnit()));
		ImGui::InputDouble("##FeedbackUnitsPerActuatorUnit", &positionFeedbackUnitsPerActuatorUnit);
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Controller");
		ImGui::PopFont();
		
		ImGui::Text("Proportional Gain");
		ImGui::InputDouble("##propGain", &proportionalGain);
		
		ImGui::Text("Derivative Gain");
		ImGui::InputDouble("##derGain", &derivativeGain);
		
		ImGui::Text("Max Following Error");
		static char maxErrorString[256];
		sprintf(maxErrorString, "%.3f %s", maxFollowingError, Unit::getDisplayStringPlural(servoActuator->positionUnit));
		ImGui::InputDouble("##maxErr", &maxFollowingError, 0.0, 0.0, maxErrorString);
		
	}
	
}

