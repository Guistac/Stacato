#include <pch.h>

#include "ActuatorToServoActuator.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "NodeGraph/Device.h"

void ActuatorToServoActuator::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Controls")){
		controlGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Settings")){
		settingsGui();
		ImGui::EndTabItem();
	}
}

void ActuatorToServoActuator::controlGui(){

	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	
	
	bool actuatorControlledExternally = isServoActuatorPinConnected();
	
	if(actuatorControlledExternally){
		ImGui::TextWrapped("Servo Actuator is Controlled by Node '%s'."
						   "\nManual controls are disabled.",
						   servoActuatorPin->getConnectedPin()->getNode()->getName());
		BEGIN_DISABLE_IMGUI_ELEMENT
	}
		
	float singleWidgetWidth = ImGui::GetContentRegionAvail().x;
	float tripleWidgetWidth = (singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0;
	glm::vec2 tripleButtonSize(tripleWidgetWidth, ImGui::GetFrameHeight());
	glm::vec2 doubleButtonSize((singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetFrameHeight());
	glm::vec2 progressBarSize(singleWidgetWidth, ImGui::GetFrameHeight());
	glm::vec2 largeDoubleButtonSize((singleWidgetWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0, ImGui::GetTextLineHeight() * 2.0);
	
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if(servoActuator->isEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		ImGui::Button("Enabled", largeDoubleButtonSize);
	}else if(servoActuator->isReady()){
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		ImGui::Button("Ready", largeDoubleButtonSize);
	}else{
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
		ImGui::Button("Disabled", largeDoubleButtonSize);
	}
	ImGui::PopStyleColor();
	ImGui::PopItemFlag();
	
	ImGui::SameLine();
	if(servoActuator->isEnabled()){
		if(ImGui::Button("Disable", largeDoubleButtonSize)) servoActuator->disable();
	}else if(servoActuator->isReady()){
		if(ImGui::Button("Enable", largeDoubleButtonSize)) servoActuator->enable();
	}else{
		BEGIN_DISABLE_IMGUI_ELEMENT
		ImGui::Button("Not Ready", largeDoubleButtonSize);
		END_DISABLE_IMGUI_ELEMENT
	}
	
	if(actuatorControlledExternally) END_DISABLE_IMGUI_ELEMENT
	
		
	bool disableManualControls = actuatorControlledExternally || !servoActuator->isEnabled();
	if(disableManualControls) BEGIN_DISABLE_IMGUI_ELEMENT

	ImGui::Text("Manual Velocity:");
	ImGui::SetNextItemWidth(singleWidgetWidth);
	ImGui::SliderFloat("##manVel", &manualVelocityDisplay, -servoActuator->getVelocityLimit(), servoActuator->getVelocityLimit());
	if (ImGui::IsItemActive()) setVelocityTarget(manualVelocityDisplay);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		manualVelocityDisplay = 0.0;
		setVelocityTarget(0.0);
	}
	
	//------------------------- POSITION CONTROLS --------------------------

	ImGui::Text("Manual Position Control");
	
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", targetPositionDisplay, Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	ImGui::InputFloat("##TargetPosition", &targetPositionDisplay, 0.0, 0.0, targetPositionString);
	
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	static char targetVelocityString[32];
	sprintf(targetVelocityString, "%.3f %s/s", targetVelocityDisplay, Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	ImGui::InputFloat("##TargetVelocity", &targetVelocityDisplay, 0.0, 0.0, targetVelocityString);
	
	ImGui::SameLine();
	ImGui::SetNextItemWidth(tripleWidgetWidth);
	ImGui::InputFloat("##TargetTime", &targetTimeDisplay, 0.0, 0.0, "%.3f s");
	
	if (ImGui::Button("Fast Move", tripleButtonSize))
		movetoPositionWithVelocity(targetPositionDisplay,
								   servoActuator->getVelocityLimit());
	
	ImGui::SameLine();
	if (ImGui::Button("Velocity Move", tripleButtonSize))
		movetoPositionWithVelocity(targetPositionDisplay,
								   targetVelocityDisplay);
	
	ImGui::SameLine();
	if (ImGui::Button("Timed Move", tripleButtonSize))
		moveToPositionInTime(targetPositionDisplay,
							 targetTimeDisplay);

	if (ImGui::Button("Stop", doubleButtonSize)) setVelocityTarget(0.0);
	
	ImGui::SameLine();
	if (ImGui::Button("Fast Stop", doubleButtonSize)) fastStop();
	
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Feedback");
	ImGui::PopFont();
	
	
	ImGui::Text("Velocity: ");
	static char velocityString[256];
	sprintf(velocityString, "%.3f %s/s", servoActuator->getVelocity(), Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	float velocityProgress = std::abs(servoActuator->getVelocityLimit() / servoActuator->getVelocityLimit());
	ImGui::ProgressBar(velocityProgress, progressBarSize, velocityString);
	
	ImGui::Text("Position: ");
	static char positionString[256];
	sprintf(positionString, "%.3f %s", servoActuator->getPosition(), Unit::getAbbreviatedString(servoActuator->getPositionUnit()));
	float positionProgress = servoActuator->getPosition() - servoActuator->getMinPosition() / (servoActuator->getMaxPosition() - servoActuator->getMinPosition());
	ImGui::ProgressBar(positionProgress, progressBarSize, positionString);
	
	if(disableManualControls) END_DISABLE_IMGUI_ELEMENT
}

void ActuatorToServoActuator::settingsGui(){
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Devices");
	ImGui::PopFont();

	std::shared_ptr<ActuatorDevice> actuatorDevice = getActuatorDevice();
	std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getPositionFeedbackDevice();
	PositionUnit positionUnit;
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Actuator Device:");
	ImGui::PopFont();
	ImGui::SameLine();
	if(isActuatorConnected()){
		positionUnit = getPositionUnit();
		if(actuatorDevice->parentDevice) ImGui::Text("%s on %s", actuatorDevice->getName(), actuatorDevice->parentDevice->getName());
		else ImGui::Text("%s on Node %s", actuatorDevice->getName(), actuatorPin->getConnectedPin()->getNode()->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	ImGui::PushFont(Fonts::robotoBold15);
	ImGui::Text("Position Feedback Device:");
	ImGui::PopFont();
	ImGui::SameLine();
	if(isPositionFeedbackConnected()){
		if(feedbackDevice->parentDevice) ImGui::Text("%s on %s", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
		else ImGui::Text("%s on Node %s", feedbackDevice->getName(), positionFeedbackPin->getConnectedPin()->getNode()->getName());
	}else ImGui::TextColored(Colors::red, "Not Connected");
	
	ImGui::Separator();
	
	if(isPositionFeedbackConnected()){
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Servo Actuator");
		ImGui::PopFont();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Actuator Position Unit: %s", Unit::getDisplayStringPlural(positionUnit));
		ImGui::Text("Actuator Velocity Limit: %.3f %s/s", actuatorDevice->getVelocityLimit(), Unit::getAbbreviatedString(positionUnit));
		ImGui::Text("Actuator Acceleration Limit: %.3f %s/s2", actuatorDevice->getAccelerationLimit(), Unit::getAbbreviatedString(positionUnit));
		ImGui::PopStyleColor();
		
		ImGui::Text("Velocity Limit");
		static char servoVelocityLimitString[256];
		sprintf(servoVelocityLimitString, "%.3f %s/s", servoActuator->velocityLimit_positionUnitsPerSecond, Unit::getAbbreviatedString(positionUnit));
		ImGui::InputDouble("##velocityLimit", &servoActuator->velocityLimit_positionUnitsPerSecond, 0.0, 0.0, servoVelocityLimitString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Text("Acceleration Limit");
		static char servoAccelerationLimitString[256];
		sprintf(servoAccelerationLimitString, "%.3f %s/s2", servoActuator->accelerationLimit_positionUnitsPerSecondSquared, Unit::getAbbreviatedString(positionUnit));
		ImGui::InputDouble("##accelerationLimit", &servoActuator->accelerationLimit_positionUnitsPerSecondSquared, 0.0, 0.0, servoAccelerationLimitString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Text("Acceleration for Manual Controls");
		static char manualAccelerationString[256];
		sprintf(manualAccelerationString, "%.3f %s/s2", manualAcceleration, Unit::getAbbreviatedString(positionUnit));
		ImGui::InputDouble("##manAcc", &manualAcceleration);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Feedback Scaling");
		ImGui::PopFont();
		
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
		ImGui::Text("Position Feedback Unit: %s", Unit::getDisplayStringPlural(feedbackDevice->getPositionUnit()));
		ImGui::PopStyleColor();
		
		ImGui::Text("Feedback %s per Actuator %s", Unit::getDisplayStringPlural(feedbackDevice->getPositionUnit()), Unit::getDisplayString(servoActuator->getPositionUnit()));
		ImGui::InputDouble("##FeedbackUnitsPerActuatorUnit", &positionFeedbackUnitsPerActuatorUnit);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Position Controller");
		ImGui::PopFont();
		
		ImGui::Text("Proportional Gain");
		ImGui::InputDouble("##propGain", &proportionalGain);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Text("Derivative Gain");
		ImGui::InputDouble("##derGain", &derivativeGain);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
		ImGui::Text("Max Following Error");
		static char maxErrorString[256];
		sprintf(maxErrorString, "%.3f %s", maxFollowingError, Unit::getDisplayStringPlural(servoActuator->positionUnit));
		ImGui::InputDouble("##maxErr", &maxFollowingError, 0.0, 0.0, maxErrorString);
		if(ImGui::IsItemDeactivatedAfterEdit()) sanitizeParameters();
		
	}
	
}

