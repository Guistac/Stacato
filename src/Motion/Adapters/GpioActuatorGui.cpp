#include <pch.h>

#include "GpioActuator.h"

#include <imgui.h>
#include <imgui_internal.h>


#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Environnement/NodeGraph/DeviceNode.h"

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
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	bool actuatorControlledExternally = isActuatorPinConnected();
	
	if(actuatorControlledExternally){
		ImGui::TextWrapped("Actuator is Controlled by Node '%s'."
						   "\nManual controls are disabled.",
						   actuatorPin->getConnectedPin()->getNode()->getName());
	}
	ImGui::BeginDisabled(actuatorControlledExternally);
		
	float singleWidgetWidth = ImGui::GetContentRegionAvail().x;
	glm::vec2 progressBarSize(singleWidgetWidth, ImGui::GetFrameHeight());
	glm::vec2 largeDoubleButtonSize((singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);
	glm::vec2 largeSingleButtonSize(singleWidgetWidth, ImGui::GetTextLineHeight() * 2.0);
	
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if(actuator->isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Enabled", largeDoubleButtonSize);
	}else if(actuator->isReady()){
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Ready", largeDoubleButtonSize);
	}else{
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Disabled", largeDoubleButtonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	
	ImGui::SameLine();
	if(actuator->isEnabled()){
		if(ImGui::Button("Disable", largeDoubleButtonSize)) actuator->disable();
	}else if(actuator->isReady()){
		if(ImGui::Button("Enable", largeDoubleButtonSize)) actuator->enable();
	}else{
		ImGui::BeginDisabled();
		ImGui::Button("Not Ready", largeDoubleButtonSize);
		ImGui::EndDisabled();
	}
	
	ImGui::EndDisabled();
	
	bool disableManualControls = actuatorControlledExternally || !actuator->isEnabled();
	ImGui::BeginDisabled(disableManualControls);
	
	ImGui::Text("Manual Velocity:");
	static char velocityCommandString[256];
	sprintf(velocityCommandString, "%.3f %s/s", manualVelocityDisplay, actuator->getPositionUnit()->abbreviated);
	ImGui::SetNextItemWidth(singleWidgetWidth);
	ImGui::SliderFloat("##manVel", &manualVelocityDisplay, -actuator->getVelocityLimit(), actuator->getVelocityLimit(), velocityCommandString);
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		manualVelocityDisplay = 0.0;
		setVelocityTarget(0.0);
	}
	
	float velocityProgress = std::abs(motionProfile.getVelocity() / actuator->getVelocityLimit());
	static char velocityString[256];
	sprintf(velocityString, "%.3f %s/s", motionProfile.getVelocity(), actuator->getPositionUnit()->abbreviated);
	ImGui::ProgressBar(velocityProgress, progressBarSize, velocityString);
	
	if(ImGui::Button("Fast Stop", largeSingleButtonSize)) fastStop();
	
	ImGui::EndDisabled();
}

void GpioActuator::settingsGui(){
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Devices");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Gpio Device: ");
	ImGui::PopFont();
	ImGui::SameLine();
	if(isGpioDeviceConnected()){
		std::shared_ptr<GpioDevice> gpioDevice = getGpioDevice();
		ImGui::Text("%s on %s", gpioDevice->getName().c_str(), gpioDevice->parentDevice->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	ImGui::Separator();
	
	if(isGpioDeviceConnected()){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Actuator Limits");
		ImGui::PopFont();
		
		ImGui::Text("Position Unit");
		if(ImGui::BeginCombo("##actuatorUnit", actuator->getPositionUnit()->singular)){
			for(auto& unit : Units::AngularDistance::get()){
				if(ImGui::Selectable(unit->singular, unit == actuator->getPositionUnit())){
					actuator->positionUnit = unit;
				}
			}
			ImGui::EndCombo();
		}
		
		ImGui::Text("Velocity Limit");
		static char servoVelocityLimitString[256];
		sprintf(servoVelocityLimitString, "%.3f %s/s", actuator->getVelocityLimit(), actuator->getPositionUnit()->abbreviated);
		ImGui::InputDouble("##velocityLimit", &actuator->velocityLimit, 0.0, 0.0, servoVelocityLimitString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		/*
		ImGui::Text("Minimum Velocity");
		static char actuatorMinVelocityString[256];
		sprintf(actuatorMinVelocityString, "%.3f %s/s", actuator->minVelocity_positionUnitsPerSecond, actuator->positionUnit->abbreviated);
		ImGui::InputDouble("##minvel", &actuator->minVelocity_positionUnitsPerSecond, 0.0, 0.0, actuatorMinVelocityString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		*/
		 
		ImGui::Text("Acceleration Limit");
		static char servoAccelerationLimitString[256];
		sprintf(servoAccelerationLimitString, "%.3f %s/s2", actuator->getAccelerationLimit(), actuator->getPositionUnit()->abbreviated);
		ImGui::InputDouble("##accelerationLimit", &actuator->accelerationLimit, 0.0, 0.0, servoAccelerationLimitString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Text("Acceleration for Manual Controls");
		static char manualAccelerationString[256];
		sprintf(manualAccelerationString, "%.3f %s/s2", manualAcceleration, actuator->positionUnit->abbreviated);
		ImGui::InputDouble("##manAcc", &manualAcceleration, 0.0, 0.0, manualAccelerationString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Control Signal");
		ImGui::PopFont();
		
		ImGui::Text("Analog Signal Range");
		ImGui::InputDouble("##outputVoltageRange", &controlSignalRange, 0.0, 0.0, "%.3f");
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Checkbox("Control Signal is Centered on Zero", &b_controlSignalIsCenteredOnZero);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Control Signal ranges from %.1f to %.1f and is centered on %.1f",
					getControlSignalLowLimit(),
					getControlSignalHighLimit(),
					getControlSignalZero());
		ImGui::PopStyleColor();
		
		ImGui::Text("Control Signal Units Per Servo Actuator %s/s", actuator->positionUnit->singular);
		ImGui::InputDouble("##UnitConversion", &controlSignalUnitsPerActuatorVelocityUnit);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Checkbox("Invert Control Signal Range", &b_invertControlSignal);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Control Signal Limits Velocity to %.1f %s/s",
					getControlSignalLimitedVelocity(),
					actuator->positionUnit->abbreviated);
		ImGui::Text("At %.1f Control Signal, Velocity is %.1f %s/s",
					getControlSignalZero(),
					controlSignalToActuatorVelocity(getControlSignalZero()),
					actuator->positionUnit->abbreviated);
		ImGui::Text("At %.1f Control Signal, Velocity is %.1f %s/s",
					getControlSignalHighLimit(),
					controlSignalToActuatorVelocity(getControlSignalHighLimit()),
					actuator->positionUnit->abbreviated);
		ImGui::PopStyleColor();
		
		ImGui::Separator();
		
		
		
		
	}
	
	
}
