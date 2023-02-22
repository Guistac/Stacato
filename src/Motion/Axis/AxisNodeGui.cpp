#include <pch.h>

#include "AxisNode.h"
#include "Motion/Interfaces.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"

void AxisNode::nodeSpecificGui(){
	
	if(ImGui::BeginTabItem("Axis")){
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Axis Feedback");
		ImGui::PopFont();
		
		ImGui::Text("Position Feedback");
		if(ImGui::BeginCombo("##pfb", positionFeedbackModule ? positionFeedbackModule->getName().c_str() : "None")){
			for(auto feedbackModule : allConnectedFeedbackModules){
				ImGui::BeginDisabled(!feedbackModule->supportsPosition());
				if(ImGui::Selectable(feedbackModule->getName().c_str(), feedbackModule == positionFeedbackModule)){
					positionFeedbackModule = feedbackModule;
				}
				ImGui::EndDisabled();
			}
			ImGui::EndCombo();
		}
		
		ImGui::TreePush();
		ImGui::Text("Position Feedback Units per Axis Units");
		ImGui::InputDouble("##pfbratio", &positionFeedbackUnitsPerAxisUnits);
		ImGui::TreePop();
		
		ImGui::Text("Velocity Feedback");
		if(ImGui::BeginCombo("##vfb", velocityFeedbackModule ? velocityFeedbackModule->getName().c_str() : "None")){
			for(auto feedbackModule : allConnectedFeedbackModules){
				ImGui::BeginDisabled(!feedbackModule->supportsVelocity());
				if(ImGui::Selectable(feedbackModule->getName().c_str(), feedbackModule == velocityFeedbackModule)){
					velocityFeedbackModule = feedbackModule;
				}
				ImGui::EndDisabled();
			}
			ImGui::EndCombo();
		}
		
		ImGui::TreePush();
		ImGui::Text("Velocity Feedback Units per Axis Units");
		ImGui::InputDouble("##vfbratio", &velocityFeedbackUnitsPerAxisUnits);
		ImGui::TreePop();
		
		ImGui::Text("Force Feedback");
		if(ImGui::BeginCombo("##ffb", forceFeedbackModule ? forceFeedbackModule->getName().c_str() : "None")){
			for(auto feedbackModule : allConnectedFeedbackModules){
				ImGui::BeginDisabled(feedbackModule->supportsForce());
				if(ImGui::Selectable(feedbackModule->getName().c_str(), feedbackModule == forceFeedbackModule)){
					forceFeedbackModule = feedbackModule;
				}
				ImGui::EndDisabled();
			}
			ImGui::EndCombo();
		}
		
		ImGui::TreePush();
		ImGui::Text("Force Feedback Units per Axis Units");
		ImGui::InputDouble("##ffbratio", &positionFeedbackUnitsPerAxisUnits);
		ImGui::TreePop();
	
		ImGui::Separator();

		
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Control Mode");
		ImGui::PopFont();
		controlModeParameter->gui();
		
		ImGui::Separator();
		
		
		if(controlModeParameter->value == controlModePosition.getInt()){
			positionControlSettingsGui();
		}else if(controlModeParameter->value == controlModeVelocity.getInt()){
			velocityControlSettingsGui();
		}else if(controlModeParameter->value == controlModeForce.getInt()){
			forceControlSettingsGui();
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

void AxisNode::positionControlSettingsGui(){
	//Option 1: at least one position actuator
	//			optionally also velocity and force actuators
	//	position commands are sent to position actuators which regulate the axis position
	//	raw velocity commands are sent to velocity actuators
	//Option 2: at least one velocity actuator and position feedback
	//			optionally also force actuators
	//	control loop with proportional gain and velocity feed forward
	//	we regulate the position by sending velocity commands
}

void AxisNode::velocityControlSettingsGui(){
	//at least one velocity actuator
	//	we send raw velocity commands to the actuators
	//optionally also force actuator
}

void AxisNode::forceControlSettingsGui(){
	//only force actuators
	//	we send raw force commands to the actuators
}

