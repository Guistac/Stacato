#include <pch.h>
#include "AxisNode.h"

#include "Motion/Interfaces.h"

OptionParameter::Option AxisNode::controlModePosition = OptionParameter::Option(1, "Position Control", "PositionControl");
OptionParameter::Option AxisNode::controlModeVelocity = OptionParameter::Option(2, "Velocity Control", "VelocityControl");
OptionParameter::Option AxisNode::controlModeForce = OptionParameter::Option(3, "Force Control", "Force Control");
std::vector<OptionParameter::Option*> AxisNode::controlModeParameterOptions = {
	&AxisNode::controlModePosition,
	&AxisNode::controlModeVelocity,
	&AxisNode::controlModeForce
};

void AxisNode::initialize(){
	axis = std::make_shared<AxisInterface>();
	
	lowerLimitSignal = std::make_shared<bool>(false);
	upperLimitSignal = std::make_shared<bool>(false);
	referenceSignal = std::make_shared<bool>(false);
	surveillanceResetSignal = std::make_shared<bool>(false);
	
	surveillanceValidSignal = std::make_shared<bool>(false);
	
	actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR_INTERFACE,
											NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
											"Actuator", "Actuator",
											NodePin::Flags::AcceptMultipleInputs);
	feedbackPin = std::make_shared<NodePin>(NodePin::DataType::MOTIONFEEDBACK_INTERFACE,
											NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
											"Feedback", "Feedback",
											NodePin::Flags::AcceptMultipleInputs);
	gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO_INTERFACE,
										NodePin::Direction::NODE_INPUT,
										"GPIO", "GPIO",
										NodePin::Flags::AcceptMultipleInputs);
	lowerLimitSignalPin = std::make_shared<NodePin>(lowerLimitSignal, NodePin::Direction::NODE_INPUT,
													"Lower Limit Signal", "LowerLimitSignal");
	upperLimitSignalPin = std::make_shared<NodePin>(upperLimitSignal, NodePin::Direction::NODE_INPUT,
													"Upper Limit Signal", "UpperLimitSignal");
	referenceSignalPin = std::make_shared<NodePin>(referenceSignal, NodePin::Direction::NODE_INPUT,
												   "Reference Signal", "ReferenceSignal");
	surveillanceResetSignalPin = std::make_shared<NodePin>(surveillanceResetSignal, NodePin::Direction::NODE_INPUT,
														   "Surveillance Fault Reset", "SurveillanceFaultReset");
	
	axisPin = std::make_shared<NodePin>(axis, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL,
										"Axis", "Axis");
	surveillanceValidSignalPin = std::make_shared<NodePin>(surveillanceValidSignal, NodePin::Direction::NODE_OUTPUT,
														   "Surveillance Valid Signal", "SurveillanceValidSignal");
	
	addNodePin(actuatorPin);
	addNodePin(feedbackPin);
	addNodePin(gpioPin);
	addNodePin(lowerLimitSignalPin);
	addNodePin(upperLimitSignalPin);
	addNodePin(referenceSignalPin);
	addNodePin(surveillanceResetSignalPin);
	
	addNodePin(axisPin);
	addNodePin(surveillanceValidSignalPin);
	
	controlModeParameter = OptionParameter::make(controlModePosition, controlModeParameterOptions, "Axis Control Mode", "AxisControlMode");
	
}

void AxisNode::onPinUpdate(std::shared_ptr<NodePin> pin){}
void AxisNode::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == actuatorPin || pin == feedbackPin || pin == gpioPin){
		updateConnectedModules();
	}
}
void AxisNode::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == actuatorPin || pin == feedbackPin || pin == gpioPin){
		updateConnectedModules();
	}
}

bool AxisNode::prepareProcess(){}
void AxisNode::inputProcess(){}
void AxisNode::outputProcess(){}
bool AxisNode::needsOutputProcess(){}
bool AxisNode::load(tinyxml2::XMLElement* xml){ return true; }
bool AxisNode::save(tinyxml2::XMLElement* xml){ return true; }


void AxisNode::updateConnectedModules(){
	
	//refresh the lists of connected modules
	connectedActuatorInterfaces.clear();
	connectedFeedbackModules.clear();
	connectedGpioInterfaces.clear();
	allConnectedFeedbackModules.clear();
	for(auto pin : actuatorPin->getConnectedPins()){
		auto actuator = pin->getSharedPointer<ActuatorInterface>();
		connectedActuatorInterfaces.push_back(actuator);
		allConnectedFeedbackModules.push_back(actuator);
	}
	for(auto pin : feedbackPin->getConnectedPins()){
		auto feedback = pin->getSharedPointer<MotionFeedbackInterface>();
		connectedFeedbackModules.push_back(feedback);
		allConnectedFeedbackModules.push_back(feedback);
	}
	for(auto pin : gpioPin->getConnectedPins()){
		auto gpio = pin->getSharedPointer<GpioInterface>();
		connectedGpioInterfaces.push_back(gpio);
	}
	
	//remove actutor control units that don't have connected actuator modules anymore
	for(int i = (int)actuatorControlUnits.size() - 1; i >= 0; i--){
		bool b_actuatorFound = false;
		for(auto actuator : connectedActuatorInterfaces){
			if(actuator == actuatorControlUnits[i]->actuator){
				b_actuatorFound = true;
				break;
			}
		}
		if(!b_actuatorFound) actuatorControlUnits.erase(actuatorControlUnits.begin() + i);
	}
	
	//add actuator control units for new connected actuators
	for(auto actuator : connectedActuatorInterfaces){
		bool b_controlUnitFound = false;
		for(auto controlUnit : actuatorControlUnits){
			if(controlUnit->actuator == actuator){
				b_controlUnitFound = true;
				break;
			}
		}
		if(!b_controlUnitFound){
			auto newControlUnit = std::make_shared<ActuatorControlUnit>(actuator);
			actuatorControlUnits.push_back(newControlUnit);
		}
	}
	
	if(positionFeedbackModule){
		bool b_positionFeedbackModuleFound = false;
		for(auto feedbackModule : allConnectedFeedbackModules){
			if(feedbackModule == positionFeedbackModule){
				b_positionFeedbackModuleFound = true;
				break;
			}
		}
		if(!b_positionFeedbackModuleFound) positionFeedbackModule = nullptr;
	}
	
	if(velocityFeedbackModule){
		bool b_velocityFeedbackModuleFound = false;
		for(auto feedbackModule : allConnectedFeedbackModules){
			if(feedbackModule == velocityFeedbackModule){
				b_velocityFeedbackModuleFound = true;
				break;
			}
		}
		if(!b_velocityFeedbackModuleFound) velocityFeedbackModule = nullptr;
	}
	
	if(forceFeedbackModule){
		bool b_forceFeedbackModuleFound = false;
		for(auto feedbackModule : allConnectedFeedbackModules){
			if(feedbackModule == forceFeedbackModule){
				b_forceFeedbackModuleFound = true;
				break;
			}
		}
		if(!b_forceFeedbackModuleFound) forceFeedbackModule = nullptr;
	}
	
}
