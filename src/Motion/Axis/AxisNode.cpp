#include <pch.h>
#include "AxisNode.h"

#include "Motion/Interfaces.h"

OptionParameter::Option AxisNode::controlModePosition = OptionParameter::Option(1, "Position Control", "PositionControl");
OptionParameter::Option AxisNode::controlModeVelocity = OptionParameter::Option(2, "Velocity Control", "VelocityControl");
std::vector<OptionParameter::Option*> AxisNode::controlModeParameterOptions = {
	&AxisNode::controlModePosition,
	&AxisNode::controlModeVelocity
};

void AxisNode::initialize(){
	axis = std::make_shared<AxisInterface>();
	
	lowerLimitSignal = std::make_shared<bool>(false);
	upperLimitSignal = std::make_shared<bool>(false);
	referenceSignal = std::make_shared<bool>(false);
	surveillanceResetSignal = std::make_shared<bool>(false);
	lowerSlowdownSignal = std::make_shared<bool>(false);
	upperSlowdownSignal = std::make_shared<bool>(false);
	
	brakeControlSignal = std::make_shared<bool>(false);
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
	lowerSlowdownSignalPin = std::make_shared<NodePin>(lowerSlowdownSignal, NodePin::Direction::NODE_INPUT,
													   "Lower Slowdown Signal", "LowerSlowdownSignal");
	upperSlowdownSignalPin = std::make_shared<NodePin>(upperSlowdownSignal, NodePin::Direction::NODE_INPUT,
													   "Upper Slowdown Signal", "UpperSlowdownSignal");
	referenceSignalPin = std::make_shared<NodePin>(referenceSignal, NodePin::Direction::NODE_INPUT,
												   "Reference Signal", "ReferenceSignal");
	surveillanceResetSignalPin = std::make_shared<NodePin>(surveillanceResetSignal, NodePin::Direction::NODE_INPUT,
														   "Surveillance Fault Reset", "SurveillanceFaultReset");
	
	axisPin = std::make_shared<NodePin>(axis, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL,
										"Axis", "Axis");
	brakeControlSignalPin = std::make_shared<NodePin>(brakeControlSignal, NodePin::Direction::NODE_OUTPUT,
													  "Brake Control Signal", "BrakeControlSignal");
	surveillanceValidSignalPin = std::make_shared<NodePin>(surveillanceValidSignal, NodePin::Direction::NODE_OUTPUT,
														   "Surveillance Valid Signal", "SurveillanceValidSignal");
	
	addNodePin(actuatorPin);
	addNodePin(feedbackPin);
	addNodePin(gpioPin);
	addNodePin(lowerLimitSignalPin);
	addNodePin(lowerSlowdownSignalPin);
	addNodePin(upperSlowdownSignalPin);
	addNodePin(upperLimitSignalPin);
	addNodePin(referenceSignalPin);
	addNodePin(surveillanceResetSignalPin);
	
	addNodePin(axisPin);
	addNodePin(brakeControlSignalPin);
	addNodePin(surveillanceValidSignalPin);
	
	controlModeParameter = OptionParameter::make(controlModePosition, controlModeParameterOptions, "Axis Control Mode", "AxisControlMode");
	
	positionLoop_velocityFeedForward = NumberParameter<double>::make(1.0, "Position loop velocity feed forward (PvFF)", "PositionLoopVelocityFeedForward");
	positionLoop_proportionalGain = NumberParameter<double>::make(0.0, "Position loop proportional gain (PKp)", "PositionLoopProportionalGain");
	positionLoop_maxError = NumberParameter<double>::make(0.0, "Position loop max error", "PositionLoopMaxError");
	positionLoop_minError = NumberParameter<double>::make(0.0, "Position loop min error", "PositionLoopMinError");
	
	hardlimitApproachVelocity = NumberParameter<double>::make(0.0, "Position loop max error", "PositionLoopMaxError");
	
	enableLowerPositionLimit = BooleanParameter::make(false, "Enable Lower Position Limit", "EnableLowerPositionLimit");
	enableUpperPositionLimit = BooleanParameter::make(false, "Enable Upper Position Limit", "EnableUpperPositionLimit");
	lowerPositionLimit = 			NumberParameter<double>::make(0.0, "Lower Position Limit", "LowerPositionLimit");
	upperPositionLimit = 			NumberParameter<double>::make(0.0, "Upper Position Limit", "UpperPositionLimit");
	lowerPositionLimitClearance = 	NumberParameter<double>::make(0.0, "Lower Position Limit Clearance", "LowerPositionLimitClearance");
	upperPositionLimitClearance = 	NumberParameter<double>::make(0.0, "Upper Position Limit Clearance", "UpperPositionLimitClearance");
	velocityLimit = 				NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit");
	accelerationLimit = 			NumberParameter<double>::make(0.0, "Acceleration Limit", "Acceleration Limit");
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

void AxisNode::inputProcess(){
	auto& fbdata = axis->feedbackProcessData;
	if(auto mapping = positionFeedbackMapping){
		fbdata.positionActual = mapping->feedbackInterface->getPosition() / mapping->feedbackUnitsPerAxisUnit;
	}
	if(auto mapping = velocityFeedbackMapping){
		fbdata.velocityActual = mapping->feedbackInterface->getVelocity() / mapping->feedbackUnitsPerAxisUnit;
	}
}

void AxisNode::outputProcess(){}
bool AxisNode::needsOutputProcess(){}


bool AxisNode::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	if(positionFeedbackMapping){
		XMLElement* pfbXML = xml->InsertNewChildElement("PositionFeedbackMapping");
		pfbXML->SetAttribute("InterfacePinID", positionFeedbackMapping->interfacePinID);
		pfbXML->SetAttribute("UnitConversion", positionFeedbackMapping->feedbackUnitsPerAxisUnit);
	}
	if(velocityFeedbackMapping){
		XMLElement* vfbXML = xml->InsertNewChildElement("VelocityFeedbackMapping");
		vfbXML->SetAttribute("InterfacePinID", velocityFeedbackMapping->interfacePinID);
		vfbXML->SetAttribute("UnitConversion", velocityFeedbackMapping->feedbackUnitsPerAxisUnit);
	}
	
	XMLElement* actuatorMappingsXML = xml->InsertNewChildElement("ActuatorMappings");
	for(auto actuatorMapping : actuatorMappings){
		XMLElement* actXML = actuatorMappingsXML->InsertNewChildElement("ActuatorMapping");
		actXML->SetAttribute("InterfacePinID", actuatorMapping->interfacePinID);
		actXML->SetAttribute("UnitConversion", actuatorMapping->actuatorUnitsPerAxisUnits);
	}
	
	return true;
}

bool AxisNode::load(tinyxml2::XMLElement* xml){
	
	return true;
}

bool AxisNode::loadAfterLinksConnected(tinyxml2::XMLElement* xml){

	using namespace tinyxml2;
	
	auto actuatorPins = actuatorPin->getConnectedPins();
	auto feedbackPins = feedbackPin->getConnectedPins();
	std::vector<std::shared_ptr<NodePin>> allFeedbackPins;
	allFeedbackPins.insert(allFeedbackPins.end(), feedbackPins.begin(), feedbackPins.end());
	allFeedbackPins.insert(allFeedbackPins.end(), actuatorPins.begin(), actuatorPins.end());
	
	if(XMLElement* pfbXML = xml->FirstChildElement("PositionFeedbackMapping")){
		int pinID;
		if(pfbXML->QueryAttribute("InterfacePinID", &pinID) != XML_SUCCESS) return false;
		for(auto fbPin : allFeedbackPins){
			if(fbPin->getUniqueID() == pinID){
				positionFeedbackMapping = std::make_shared<FeedbackMapping>(fbPin);
				if(pfbXML->QueryAttribute("UnitConversion", &positionFeedbackMapping->feedbackUnitsPerAxisUnit) != XML_SUCCESS) return false;
				break;
			}
		}
	}
	
	if(XMLElement* vfbXML = xml->FirstChildElement("VelocityFeedbackMapping")){
		int pinID;
		if(vfbXML->QueryAttribute("InterfacePinID", &pinID) != XML_SUCCESS) return false;
		for(auto fbPin : allFeedbackPins){
			if(fbPin->getUniqueID() == pinID){
				velocityFeedbackMapping = std::make_shared<FeedbackMapping>(fbPin);
				if(vfbXML->QueryAttribute("UnitConversion", &velocityFeedbackMapping->feedbackUnitsPerAxisUnit) != XML_SUCCESS) return false;
				break;
			}
		}
	}
	
	if(XMLElement* actuatorMappingsXML = xml->FirstChildElement("ActuatorMappings")){
		XMLElement* actXML = actuatorMappingsXML->FirstChildElement("ActuatorMapping");
		while(actXML){
			int interfacePinID;
			if(actXML->QueryAttribute("InterfacePinID", &interfacePinID) != XML_SUCCESS) return false;
			for(auto mapping : actuatorMappings){
				if(mapping->interfacePinID == interfacePinID){
					if(actXML->QueryAttribute("UnitConversion", &mapping->actuatorUnitsPerAxisUnits) != XML_SUCCESS) return false;
					break;
				}
			}
			actXML = actXML->NextSiblingElement("ActuatorMapping");
		}
	}
	

	return true;
}


void AxisNode::updateConnectedModules(){
	
	auto actuatorPins = actuatorPin->getConnectedPins();
	auto feedbackPins = feedbackPin->getConnectedPins();
	std::vector<std::shared_ptr<NodePin>> allFeedbackPins;
	allFeedbackPins.insert(allFeedbackPins.end(), feedbackPins.begin(), feedbackPins.end());
	allFeedbackPins.insert(allFeedbackPins.end(), actuatorPins.begin(), actuatorPins.end());
	
	//remove actutor control units that don't have connected actuator modules anymore
	for(int i = (int)actuatorMappings.size() - 1; i >= 0; i--){
		bool b_actuatorFound = false;
		for(auto actuatorPin : actuatorPin->getConnectedPins()){
			if(actuatorPin->getUniqueID() == actuatorMappings[i]->interfacePinID){
				b_actuatorFound = true;
				break;
			}
		}
		if(!b_actuatorFound) actuatorMappings.erase(actuatorMappings.begin() + i);
	}
	
	//add actuator control units for new connected actuators
	for(auto actuatorPin : actuatorPin->getConnectedPins()){
		bool b_interfaceFound = false;
		for(auto actuatorMapping : actuatorMappings){
			if(actuatorPin->getUniqueID() == actuatorMapping->interfacePinID){
				b_interfaceFound = true;
				break;
			}
		}
		if(!b_interfaceFound){
			auto newMapping = std::make_shared<ActuatorMapping>(actuatorPin);
			actuatorMappings.push_back(newMapping);
		}
	}
	
	if(positionFeedbackMapping){
		bool b_interfaceFound = false;
		for(auto feedbackPin : allFeedbackPins){
			if(feedbackPin->getUniqueID() == positionFeedbackMapping->interfacePinID){
				b_interfaceFound = true;
				break;
			}
		}
		if(!b_interfaceFound) positionFeedbackMapping = nullptr;
	}
	
	if(velocityFeedbackMapping){
		bool b_interfaceFound = false;
		for(auto feedbackPin : allFeedbackPins){
			if(feedbackPin->getUniqueID() == velocityFeedbackMapping->interfacePinID){
				b_interfaceFound = true;
				break;
			}
		}
		if(!b_interfaceFound) velocityFeedbackMapping = nullptr;
	}
	
	connectedDeviceInterfaces.clear();
	for(auto actuatorInterfacePin : actuatorPin->getConnectedPins()){
		auto actuator = actuatorInterfacePin->getSharedPointer<ActuatorInterface>();
		connectedDeviceInterfaces.push_back(actuator);
	}
	for(auto feedbackInterfacePin : feedbackPin->getConnectedPins()){
		auto feedback = feedbackInterfacePin->getSharedPointer<MotionFeedbackInterface>();
		connectedDeviceInterfaces.push_back(feedback);
	}
	for(auto gpioInterfacePin : gpioPin->getConnectedPins()){
		auto gpio = gpioInterfacePin->getSharedPointer<GpioInterface>();
		connectedDeviceInterfaces.push_back(gpio);
	}
	
}
