#include <pch.h>

#include "AxisNode.h"
#include "Motion/Interfaces.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"

#include "Gui/Utilities/CustomWidgets.h"

void AxisNode::nodeSpecificGui(){
	controlTab();
	configurationTab();
	devicesTab();
}

void AxisNode::controlTab(){
	if(ImGui::BeginTabItem("Control")){
		
		ImVec2 buttonSize(ImGui::GetContentRegionAvail().x / 2.0, ImGui::GetTextLineHeight() * 2.0);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::BeginDisabled(!axisInterface->isOnline());
		if(axisInterface->isEnabled()){
			if(customButton("Disable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
				axisInterface->disable();
			}
		}else{
			if(customButton("Enable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
				axisInterface->enable();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine(0.0, 0.0);
		backgroundText("State", buttonSize, Colors::gray, Colors::black, ImDrawFlags_RoundCornersRight);
		ImGui::PopFont();
		
		ImVec2 progressBarSize(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5);
		
		if(axisInterface->configuration.b_supportsPositionFeedback){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Position Feedback");
			ImGui::PopFont();
			double pos = positionFeedbackMapping->feedbackInterface->getPosition() / positionFeedbackMapping->feedbackUnitsPerAxisUnit;
			std::ostringstream positionString;
			positionString << std::fixed << std::setprecision(3) << pos << " u";
			ImGui::ProgressBar(axisInterface->getPositionNormalizedToLimits(), progressBarSize, positionString.str().c_str());
		}
		if(axisInterface->configuration.b_supportsVelocityFeedback){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Velocity Feedback");
			ImGui::PopFont();
			double vel = velocityFeedbackMapping->feedbackInterface->getVelocity() / velocityFeedbackMapping->feedbackUnitsPerAxisUnit;
			std::ostringstream velocityString;
			velocityString << std::fixed << std::setprecision(3) << vel << " u/s";
			ImGui::ProgressBar(axisInterface->getVelocityNormalizedToLimits(), progressBarSize, velocityString.str().c_str());
		}
		if(axisInterface->configuration.b_supportsEffortFeedback){
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("Effort Feedback");
			ImGui::PopFont();
			
			double axisEffort = axisInterface->processData.effortActual;
			std::ostringstream axisEffortString;
			axisEffortString << "Axis : " << std::fixed << std::setprecision(1) << axisEffort * 100.0 << "%";
			ImGui::ProgressBar(axisEffort, progressBarSize, axisEffortString.str().c_str());
			
			for(int i = 0; i < actuatorMappings.size(); i++){
				auto actuator = actuatorMappings[i]->actuatorInterface;
				double actuatorEffort = actuator->getEffort();
				std::ostringstream actuatorEffortString;
				actuatorEffortString << actuator->getName() << " : " << std::fixed << std::setprecision(1) << actuatorEffort * 100.0 << "%";
				ImGui::ProgressBar(actuatorEffort, progressBarSize, actuatorEffortString.str().c_str());
			}
			
		}
		if(axisInterface->configuration.b_supportsHoming){
			
		}
		
		
		
		for(auto mapping : actuatorMappings){
			ImGui::Text("%s %.3f", mapping->actuatorInterface->getName().c_str(), mapping->actuatorPositionOffset);
		}
		
		
		ImGui::SliderFloat("##vel", &manualVelocityTarget, -axisInterface->getVelocityLimit(), axisInterface->getVelocityLimit());
		if(ImGui::IsItemDeactivatedAfterEdit()) manualVelocityTarget = 0.0;
		ImGui::InputFloat("##acc", &manualVelocityAcceleration);
		ImGui::EndTabItem();
	}
}

void AxisNode::configurationTab(){
	if(ImGui::BeginTabItem("Configuration")){
		
		controlModeParameter->gui(Fonts::sansBold15);
		limitSignalTypeParameter->gui(Fonts::sansBold15);
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Motion Feedback")){
			ImGui::PopFont();
			motionFeedbackSettingsGui();
		}else ImGui::PopFont();
		
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Actuator Control Modes")){
			ImGui::PopFont();
			actuatorControlSettingsGui();
		}else ImGui::PopFont();
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Limits")){
			ImGui::PopFont();
			limitSettingsGui();
		}else ImGui::PopFont();
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Position Control")){
			ImGui::PopFont();
			positionControlSettingsGui();
		}else ImGui::PopFont();
		
		if(controlModeParameter->value == ControlMode::POSITION_CONTROL){
			ImGui::PushFont(Fonts::sansBold20);
			if(ImGui::CollapsingHeader("Homing")){
				ImGui::PopFont();
				homingSettingsGui();
			}else ImGui::PopFont();
		}
		
		ImGui::PushFont(Fonts::sansBold20);
		if(ImGui::CollapsingHeader("Miscellaneous")){
			ImGui::PopFont();
			maxEnableTimeSeconds->gui(Fonts::sansBold15);
		}else ImGui::PopFont();
		
		ImGui::EndTabItem();
	}
}

void AxisNode::motionFeedbackSettingsGui(){
	ImGui::Text("Position Feedback");
	std::string pfbMappingString = positionFeedbackMapping ? positionFeedbackMapping->feedbackInterface->getName() : "None";
	if(ImGui::BeginCombo("##pfb", pfbMappingString.c_str())){
		for(auto connectedFeedbackPin : feedbackPin->getConnectedPins()){
			auto feedbackInterface = connectedFeedbackPin->getSharedPointer<MotionFeedbackInterface>();
			bool b_selected = positionFeedbackMapping && positionFeedbackMapping->feedbackInterface == feedbackInterface;
			ImGui::BeginDisabled(!feedbackInterface->supportsPosition());
			if(ImGui::Selectable(feedbackInterface->getName().c_str(), b_selected)){
				positionFeedbackMapping = std::make_shared<FeedbackMapping>(connectedFeedbackPin);
				updateAxisConfiguration();
			}
			ImGui::EndDisabled();
		}
		for(auto connectedActuatorPin : actuatorPin->getConnectedPins()){
			auto feedbackInterface = connectedActuatorPin->getSharedPointer<MotionFeedbackInterface>();
			bool b_selected = positionFeedbackMapping && positionFeedbackMapping->feedbackInterface == feedbackInterface;
			ImGui::BeginDisabled(!feedbackInterface->supportsPosition());
			if(ImGui::Selectable(feedbackInterface->getName().c_str(), b_selected)){
				positionFeedbackMapping = std::make_shared<FeedbackMapping>(connectedActuatorPin);
				updateAxisConfiguration();
			}
			ImGui::EndDisabled();
		}
		ImGui::EndCombo();
	}
	
	if(positionFeedbackMapping){
		ImGui::TreePush();
		ImGui::Text("Position Feedback Units per Axis Units");
		ImGui::InputDouble("##pfbratio", &positionFeedbackMapping->feedbackUnitsPerAxisUnit);
		ImGui::TreePop();
	}
	
	
	ImGui::Text("Velocity Feedback");
	std::string vfbMappingString = velocityFeedbackMapping ? velocityFeedbackMapping->feedbackInterface->getName() : "None";
	if(ImGui::BeginCombo("##vfb", vfbMappingString.c_str())){
		for(auto connectedFeedbackPin : feedbackPin->getConnectedPins()){
			auto feedbackInterface = connectedFeedbackPin->getSharedPointer<MotionFeedbackInterface>();
			bool b_selected = velocityFeedbackMapping && velocityFeedbackMapping->feedbackInterface == feedbackInterface;
			ImGui::BeginDisabled(!feedbackInterface->supportsVelocity());
			if(ImGui::Selectable(feedbackInterface->getName().c_str(), b_selected)){
				velocityFeedbackMapping = std::make_shared<FeedbackMapping>(connectedFeedbackPin);
			}
			ImGui::EndDisabled();
		}
		for(auto connectedActuatorPin : actuatorPin->getConnectedPins()){
			auto feedbackInterface = connectedActuatorPin->getSharedPointer<MotionFeedbackInterface>();
			bool b_selected = velocityFeedbackMapping && velocityFeedbackMapping->feedbackInterface == feedbackInterface;
			ImGui::BeginDisabled(!feedbackInterface->supportsVelocity());
			if(ImGui::Selectable(feedbackInterface->getName().c_str(), b_selected)){
				velocityFeedbackMapping = std::make_shared<FeedbackMapping>(connectedActuatorPin);
			}
			ImGui::EndDisabled();
		}
		ImGui::EndCombo();
	}
	
	if(velocityFeedbackMapping){
		ImGui::TreePush();
		ImGui::Text("Velocity Feedback Units per Axis Units");
		ImGui::InputDouble("##vfbratio", &velocityFeedbackMapping->feedbackUnitsPerAxisUnit);
		ImGui::TreePop();
	}
}

void AxisNode::actuatorControlSettingsGui(){
	for(int i = 0; i < actuatorMappings.size(); i++){
		auto mapping = actuatorMappings[i];
		auto actuator = mapping->actuatorInterface;
		ImGui::PushID(i);
		
		ImGui::PushFont(Fonts::sansBold15);
		backgroundText(actuator->getName().c_str(), Colors::darkGray, Colors::white);
		ImGui::PopFont();
	
		ImGui::TreePush();
		ImGui::Text("Actuator units per axis units");
		ImGui::InputDouble("##conv", &mapping->actuatorUnitsPerAxisUnits);
		std::string controlModeString;
		switch(mapping->controlModeSelection){
			case ActuatorInterface::ControlMode::POSITION:
				controlModeString = "Position Control";
				break;
			case ActuatorInterface::ControlMode::VELOCITY:
				controlModeString = "Velocity Control";
				break;
			case ActuatorInterface::ControlMode::FORCE:
				controlModeString = "Force Control";
				break;
		}
		
		ImGui::Text("Actuator Control Mode");
		if(ImGui::BeginCombo("##cmode", controlModeString.c_str())){
			if(actuator->supportsPositionControl() &&
			   ImGui::Selectable("Position Control", mapping->controlModeSelection == ActuatorInterface::ControlMode::POSITION)){
				mapping->controlModeSelection = ActuatorInterface::ControlMode::POSITION;
			}
			if(actuator->supportsVelocityControl() &&
			   ImGui::Selectable("Velocity Control", mapping->controlModeSelection == ActuatorInterface::ControlMode::VELOCITY)){
				mapping->controlModeSelection = ActuatorInterface::ControlMode::VELOCITY;
			}
			if(actuator->supportsForceControl() &&
			   ImGui::Selectable("Force Control", mapping->controlModeSelection == ActuatorInterface::ControlMode::FORCE)){
				mapping->controlModeSelection = ActuatorInterface::ControlMode::FORCE;
			}
			ImGui::EndCombo();
		}
		ImGui::TreePop();
		ImGui::PopID();
	}
}

void AxisNode::limitSettingsGui(){

	velocityLimit->gui(Fonts::sansBold15);
	accelerationLimit->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	ImGui::BeginDisabled(controlModeParameter->value != controlModePosition.getInt());
	
	enableLowerPositionLimit->gui(Fonts::sansBold15);
	lowerPositionLimit->gui(Fonts::sansBold15);
	lowerPositionLimitClearance->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	enableUpperPositionLimit->gui(Fonts::sansBold15);
	upperPositionLimit->gui(Fonts::sansBold15);
	upperPositionLimitClearance->gui(Fonts::sansBold15);
	
	ImGui::EndDisabled();
}

void AxisNode::positionControlSettingsGui(){
	
	ImGui::BeginDisabled(controlModeParameter->value != ControlMode::POSITION_CONTROL);
	positionLoop_velocityFeedForward->gui(Fonts::sansBold15);
	positionLoop_proportionalGain->gui(Fonts::sansBold15);
	positionLoop_maxError->gui(Fonts::sansBold15);
	positionLoop_minError->gui(Fonts::sansBold15);
	ImGui::EndDisabled();
	
	ImGui::BeginDisabled(limitSignalTypeParameter->value != LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS);
	limitSlowdownVelocity->gui(Fonts::sansBold15);
	ImGui::EndDisabled();
}

void AxisNode::homingSettingsGui(){
	homingDirectionParameter->gui(Fonts::sansBold15);
	signalApproachParameter->gui(Fonts::sansBold15);
	homingVelocityCoarse->gui(Fonts::sansBold15);
	homingVelocityFine->gui(Fonts::sansBold15);
	maxHomingDistanceCoarse->gui(Fonts::sansBold15);
	maxHomingDistanceFine->gui(Fonts::sansBold15);
}


void AxisNode::devicesTab(){
	if(ImGui::BeginTabItem("Devices")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Connected Devices");
		ImGui::PopFont();
		
		for(auto device : connectedDeviceInterfaces){
			backgroundText(device->getName().c_str(), ImVec2(0,0), Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersLeft);
			glm::vec4 deviceStatusColor;
			std::string deviceStatusString;
			switch(device->getState()){
				case DeviceState::OFFLINE:
					deviceStatusColor = Colors::blue;
					deviceStatusString = "Offline";
					break;
				case DeviceState::NOT_READY:
					deviceStatusColor = Colors::red;
					deviceStatusString = "Not Ready";
					break;
				case DeviceState::READY:
					deviceStatusColor = Colors::yellow;
					deviceStatusString = "Ready";
					break;
				case DeviceState::ENABLING:
					deviceStatusColor = Colors::yellow;
					deviceStatusString = "Enabling...";
					break;
				case DeviceState::DISABLING:
					deviceStatusColor = Colors::yellow;
					deviceStatusString = "Disabling...";
					break;
					break;
				case DeviceState::ENABLED:
					deviceStatusColor = Colors::green;
					deviceStatusString = "Enabled";
					break;
			}
			ImGui::SameLine(0.0,0.0);
			backgroundText(deviceStatusString.c_str(), ImVec2(0,0), deviceStatusColor, Colors::black, ImDrawFlags_RoundCornersRight);
			ImGui::TreePush();
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("%s", device->getStatusString().c_str());
			ImGui::PopStyleColor();
			ImGui::TreePop();
		}
		
		ImGui::EndTabItem();
	}
}
