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

void AxisNode::positionControlSettingsGui(){
	
	velocityLimit->gui(Fonts::sansBold15);
	accelerationLimit->gui(Fonts::sansBold15);
	
	positionLoop_velocityFeedForward->gui(Fonts::sansBold15);
	positionLoop_proportionalGain->gui(Fonts::sansBold15);
	positionLoop_maxError->gui(Fonts::sansBold15);
	positionLoop_minError->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Limits");
	ImGui::PopFont();

	enableLowerPositionLimit->gui(Fonts::sansBold15);
	lowerPositionLimit->gui(Fonts::sansBold15);
	lowerPositionLimitClearance->gui(Fonts::sansBold15);
	
	enableUpperPositionLimit->gui(Fonts::sansBold15);
	upperPositionLimit->gui(Fonts::sansBold15);
	upperPositionLimitClearance->gui(Fonts::sansBold15);
	
}

void AxisNode::velocityControlSettingsGui(){
	
	hardlimitApproachVelocity->gui(Fonts::sansBold15);
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Limits");
	ImGui::PopFont();

	velocityLimit->gui(Fonts::sansBold15);
	accelerationLimit->gui(Fonts::sansBold15);
	
}



void AxisNode::controlTab(){
	if(ImGui::BeginTabItem("Control")){
		
		ImGui::EndTabItem();
	}
}

void AxisNode::configurationTab(){
	if(ImGui::BeginTabItem("Configuration")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Motion Feedback");
		ImGui::PopFont();
		
		ImGui::Text("Position Feedback");
		std::string pfbMappingString = positionFeedbackMapping ? positionFeedbackMapping->feedbackInterface->getName() : "None";
		if(ImGui::BeginCombo("##pfb", pfbMappingString.c_str())){
			for(auto connectedFeedbackPin : feedbackPin->getConnectedPins()){
				auto feedbackInterface = connectedFeedbackPin->getSharedPointer<MotionFeedbackInterface>();
				bool b_selected = positionFeedbackMapping && positionFeedbackMapping->feedbackInterface == feedbackInterface;
				ImGui::BeginDisabled(!feedbackInterface->supportsPosition());
				if(ImGui::Selectable(feedbackInterface->getName().c_str(), b_selected)){
					positionFeedbackMapping = std::make_shared<FeedbackMapping>(connectedFeedbackPin);
				}
				ImGui::EndDisabled();
			}
			for(auto connectedActuatorPin : actuatorPin->getConnectedPins()){
				auto feedbackInterface = connectedActuatorPin->getSharedPointer<MotionFeedbackInterface>();
				bool b_selected = positionFeedbackMapping && positionFeedbackMapping->feedbackInterface == feedbackInterface;
				ImGui::BeginDisabled(!feedbackInterface->supportsPosition());
				if(ImGui::Selectable(feedbackInterface->getName().c_str(), b_selected)){
					positionFeedbackMapping = std::make_shared<FeedbackMapping>(connectedActuatorPin);
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
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Control Mode");
		ImGui::PopFont();
		controlModeParameter->gui();
		
		if(controlModeParameter->value == controlModePosition.getInt()){
			positionControlSettingsGui();
		}else if(controlModeParameter->value == controlModeVelocity.getInt()){
			velocityControlSettingsGui();
		}
		
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Actuator Control");
		ImGui::PopFont();
		
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
		
		
		/*
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Actuator Control");
		ImGui::PopFont();
		
		for(int i = 0; i < actuatorControlUnits.size(); i++){
			ImGui::PushID(i);
			auto controlUnit = actuatorControlUnits[i];
			auto actuator = controlUnit->actuator;
			ImGui::PushFont(Fonts::sansBold15);
			ImGui::Text("%s", actuator->getName().c_str());
			ImGui::PopFont();
			ImGui::TreePush();
			std::string controlModeString;
			switch(controlUnit->controlModeSelection){
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
				   ImGui::Selectable("Position Control", controlUnit->controlModeSelection == ActuatorInterface::ControlMode::POSITION)){
					controlUnit->controlModeSelection = ActuatorInterface::ControlMode::POSITION;
				}
				if(actuator->supportsVelocityControl() &&
				   ImGui::Selectable("Velocity Control", controlUnit->controlModeSelection == ActuatorInterface::ControlMode::VELOCITY)){
					controlUnit->controlModeSelection = ActuatorInterface::ControlMode::VELOCITY;
				}
				if(actuator->supportsForceControl() &&
				   ImGui::Selectable("Force Control", controlUnit->controlModeSelection == ActuatorInterface::ControlMode::FORCE)){
					controlUnit->controlModeSelection = ActuatorInterface::ControlMode::FORCE;
				}
				ImGui::EndCombo();
			}
			
			ImGui::Text("Actuator Units Per Axis Units");
			ImGui::InputDouble("##ratio", &controlUnit->actuatorUnitsPerAxisUnits);
			
			ImGui::TreePop();
			ImGui::PopID();
		}
		*/
		
		/*
		ImGui::Separator();
		
		enableSurveillanceParameter->gui();
		ImGui::SameLine();
		ImGui::Text("Surveillance is%s enabled", enableSurveillanceParameter->value ? "" : " not");
		
		std::string surveillanceModuleName = "No Device";
		if(surveillanceFeedbackModule) surveillanceModuleName = surveillanceFeedbackModule->getName();
		if(ImGui::BeginCombo("##srv", surveillanceModuleName.c_str())){
			for(auto feedbackModule : connectedFeedbackModules){
				ImGui::BeginDisabled(feedbackModule == masterFeedbackModule);
				if(ImGui::Selectable(feedbackModule->getName().c_str(), feedbackModule == surveillanceFeedbackModule)){
					surveillanceFeedbackModule = feedbackModule;
				}
				ImGui::EndDisabled();
			}
			ImGui::EndCombo();
		}
		 */
		
		ImGui::EndTabItem();
	}
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
