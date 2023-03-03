#include <pch.h>
#include "AxisNode.h"

#include "Motion/Interfaces.h"

OptionParameter::Option AxisNode::controlModePosition = OptionParameter::Option(0, "Position Control", "PositionControl");
OptionParameter::Option AxisNode::controlModeVelocity = OptionParameter::Option(1, "Velocity Control", "VelocityControl");
OptionParameter::Option AxisNode::controlModeNone =		OptionParameter::Option(2, "No Control", "NoControl");
std::vector<OptionParameter::Option*> AxisNode::controlModeParameterOptions = {
	&AxisNode::controlModePosition,
	&AxisNode::controlModeVelocity,
	&AxisNode::controlModeNone
};

OptionParameter::Option AxisNode::option_NoLimitSignal =
	OptionParameter::Option(0, "No limit signal", "NoLimitSignal");
OptionParameter::Option AxisNode::option_SignalAtLowerLimit =
	OptionParameter::Option(1, "Signal at lower limit", "SignalAtLowerLimit");
OptionParameter::Option AxisNode::option_SignalAtLowerAndUpperLimits =
	OptionParameter::Option(2, "Signal at lower and upper limit", "SignalAtLowerAndUpperLimit");
OptionParameter::Option AxisNode::option_SignalAtOrigin =
	OptionParameter::Option(3, "Signal at origin", "SignalAtOrigin");
OptionParameter::Option AxisNode::option_LimitAndSlowdownAtLowerAndUpperLimits =
	OptionParameter::Option(4, "Limit and slowdown signals", "LimitAndSlowdownSignals");

std::vector<OptionParameter::Option*> AxisNode::limitSignalTypeOptions = {
	&option_NoLimitSignal,
	&option_SignalAtLowerLimit,
	&option_SignalAtLowerAndUpperLimits,
	&option_SignalAtOrigin,
	&option_LimitAndSlowdownAtLowerAndUpperLimits
};

 
OptionParameter::Option AxisNode::option_HomingDirectionNegative = OptionParameter::Option(0, "Negative", "Negative");
OptionParameter::Option AxisNode::option_HomingDirectionPositive = OptionParameter::Option(1, "Positive", "Positive");
std::vector<OptionParameter::Option*> AxisNode::homingDirectionOptions = {
	&option_HomingDirectionNegative,
	&option_HomingDirectionPositive
};

OptionParameter::Option AxisNode::option_FindSignalEdge = OptionParameter::Option(0, "Find signal edge", "FindSignalEdge");
OptionParameter::Option AxisNode::option_FindSignalCenter = OptionParameter::Option(1, "Find signal center", "FindSignalCenter");
std::vector<OptionParameter::Option*> AxisNode::signalApproachOptions = {
	&option_FindSignalEdge,
	&option_FindSignalCenter
};


void AxisNode::initialize(){
	axisInterface = std::make_shared<AxisInterface>();
	
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
	
	axisPin = std::make_shared<NodePin>(axisInterface, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL,
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
	controlModeParameter->addEditCallback([this](){ updateControlMode(); });
	 
	maxEnableTimeSeconds = NumberParameter<double>::make(0.1, "Max enable time (seconds)", "MaxEnableTime");
	
	positionLoop_velocityFeedForward = NumberParameter<double>::make(1.0, "Position loop velocity feed forward (PvFF)", "PositionLoopVelocityFeedForward");
	positionLoop_proportionalGain = NumberParameter<double>::make(0.0, "Position loop proportional gain (PKp)", "PositionLoopProportionalGain");
	positionLoop_maxError = NumberParameter<double>::make(0.0, "Position loop max error", "PositionLoopMaxError");
	positionLoop_minError = NumberParameter<double>::make(0.0, "Position loop min error", "PositionLoopMinError");
	
	velocityLoop_maxError = NumberParameter<double>::make(0.0, "Velocity loop max error", "VelocityLoopMaxError");
	limitSlowdownVelocity = NumberParameter<double>::make(0.0, "Limit Slowdown Velocity", "LimitSlowdownVelocity");
	
	enableLowerPositionLimit = BooleanParameter::make(false, "Enable Lower Position Limit", "EnableLowerPositionLimit");
	enableUpperPositionLimit = BooleanParameter::make(false, "Enable Upper Position Limit", "EnableUpperPositionLimit");
	lowerPositionLimit = 			NumberParameter<double>::make(0.0, "Lower Position Limit", "LowerPositionLimit");
	upperPositionLimit = 			NumberParameter<double>::make(0.0, "Upper Position Limit", "UpperPositionLimit");
	lowerPositionLimitClearance = 	NumberParameter<double>::make(0.0, "Lower Position Limit Clearance", "LowerPositionLimitClearance");
	upperPositionLimitClearance = 	NumberParameter<double>::make(0.0, "Upper Position Limit Clearance", "UpperPositionLimitClearance");
	velocityLimit = 				NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit");
	accelerationLimit = 			NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit");
	
	auto updateInterfaceCallback = [this](){updateAxisConfiguration();};
	enableLowerPositionLimit->addEditCallback(updateInterfaceCallback);
	enableUpperPositionLimit->addEditCallback(updateInterfaceCallback);
	lowerPositionLimit->addEditCallback(updateInterfaceCallback);
	upperPositionLimit->addEditCallback(updateInterfaceCallback);
	lowerPositionLimitClearance->addEditCallback(updateInterfaceCallback);
	upperPositionLimitClearance->addEditCallback(updateInterfaceCallback);
	velocityLimit->addEditCallback(updateInterfaceCallback);
	accelerationLimit->addEditCallback(updateInterfaceCallback);
	
	
	
	
	homingDirectionParameter = 	OptionParameter::make(option_HomingDirectionNegative, homingDirectionOptions, "Homing direction", "HomingDirection");
	signalApproachParameter = 	OptionParameter::make(option_FindSignalEdge, signalApproachOptions, "Signal approach method", "SignalApproachMethod");
	homingVelocityCoarse = 		NumberParameter<double>::make(0.0, "Homing velocity coarse", "HomingVelocityCoarse");
	homingVelocityFine = 		NumberParameter<double>::make(0.0, "Homing velocity fine", "HomingVelocityFine");
	maxHomingDistanceCoarse = 	NumberParameter<double>::make(0.0, "Max homing distance coarse", "MaxHomingDistanceCoarse");
	maxHomingDistanceFine = 	NumberParameter<double>::make(0.0, "Max homing distance fine", "MaxHomingDistanceFine");
	
	limitSignalTypeParameter = OptionParameter::make(option_SignalAtLowerLimit, limitSignalTypeOptions, "Limit Signal Type", "LimitSignalType");
	limitSignalTypeParameter->addEditCallback([this](){ updateLimitSignalType(); });
	
	updateControlMode();
	updateLimitSignalType();
	updateAxisConfiguration();
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


bool AxisNode::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	if(positionFeedbackMapping){
		XMLElement* pfbXML = xml->InsertNewChildElement("PositionFeedbackMapping");
		pfbXML->SetAttribute("InterfacePinID", positionFeedbackMapping->interfacePinID);
		positionFeedbackMapping->feedbackUnitsPerAxisUnit->save(pfbXML);
	}
	if(velocityFeedbackMapping){
		XMLElement* vfbXML = xml->InsertNewChildElement("VelocityFeedbackMapping");
		vfbXML->SetAttribute("InterfacePinID", velocityFeedbackMapping->interfacePinID);
		positionFeedbackMapping->feedbackUnitsPerAxisUnit->save(vfbXML);
	}
	
	XMLElement* actuatorMappingsXML = xml->InsertNewChildElement("ActuatorMappings");
	for(auto actuatorMapping : actuatorMappings){
		XMLElement* actXML = actuatorMappingsXML->InsertNewChildElement("ActuatorMapping");
		actXML->SetAttribute("InterfacePinID", actuatorMapping->interfacePinID);
		actuatorMapping->actuatorUnitsPerAxisUnits->save(actXML);
		actuatorMapping->controlModeParameter->save(actXML);
	}
	
	velocityLimit->save(xml);
	accelerationLimit->save(xml);
	
	positionLoop_velocityFeedForward->save(xml);
	positionLoop_proportionalGain->save(xml);
	positionLoop_maxError->save(xml);
	positionLoop_minError->save(xml);
	velocityLoop_maxError->save(xml);
	limitSlowdownVelocity->save(xml);
	
	enableLowerPositionLimit->save(xml);
	enableUpperPositionLimit->save(xml);
	lowerPositionLimit->save(xml);
	upperPositionLimit->save(xml);
	lowerPositionLimitClearance->save(xml);
	upperPositionLimitClearance->save(xml);
	velocityLimit->save(xml);
	accelerationLimit->save(xml);
	
	return true;
}

bool AxisNode::load(tinyxml2::XMLElement* xml){
	
	bool success = true;
	success &= velocityLimit->load(xml);
	success &= accelerationLimit->load(xml);
	success &= positionLoop_velocityFeedForward->load(xml);
	success &= positionLoop_proportionalGain->load(xml);
	success &= positionLoop_maxError->load(xml);
	success &= positionLoop_minError->load(xml);
	success &= velocityLoop_maxError->load(xml);
	success &= limitSlowdownVelocity->load(xml);
	
	
	success &= enableLowerPositionLimit->load(xml);
	success &= enableUpperPositionLimit->load(xml);
	success &= lowerPositionLimit->load(xml);
	success &= upperPositionLimit->load(xml);
	success &= lowerPositionLimitClearance->load(xml);
	success &= upperPositionLimitClearance->load(xml);
	success &= velocityLimit->load(xml);
	success &= accelerationLimit->load(xml);
	
	manualAccelerationEntry = accelerationLimit->value;
		
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
				if(!positionFeedbackMapping->feedbackUnitsPerAxisUnit->load(pfbXML)) return false;
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
				if(!velocityFeedbackMapping->feedbackUnitsPerAxisUnit->load(vfbXML)) return false;
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
					if(!mapping->actuatorUnitsPerAxisUnits->load(actXML)) return false;
					if(!mapping->controlModeParameter->load(actXML)) return false;
					mapping->controlModeParameter->onEdit();
					break;
				}
			}
			actXML = actXML->NextSiblingElement("ActuatorMapping");
		}
	}
	
	updateAxisConfiguration();
	

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
	
	/*
	positionFeedbackOptions.clear();
	for(auto feedbackPin : allFeedbackPins){
		int id = feedbackPin->uniqueID;
	}
	positionFeedbackOptions.push_back(&noPositionFeedbackOption);
	
	velocityFeedbackOptions.clear();
	velocityFeedbackOptions.push_back(&noVelocityFeedbackOption);
	*/
	
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
	connectedActuatorInterfaces.clear();
	connectedFeedbackInteraces.clear();
	connectedGpioInterfaces.clear();
	for(auto actuatorInterfacePin : actuatorPin->getConnectedPins()){
		auto actuator = actuatorInterfacePin->getSharedPointer<ActuatorInterface>();
		connectedDeviceInterfaces.push_back(actuator);
		connectedActuatorInterfaces.push_back(actuator);
	}
	for(auto feedbackInterfacePin : feedbackPin->getConnectedPins()){
		auto feedback = feedbackInterfacePin->getSharedPointer<MotionFeedbackInterface>();
		connectedDeviceInterfaces.push_back(feedback);
		connectedFeedbackInteraces.push_back(feedback);
	}
	for(auto gpioInterfacePin : gpioPin->getConnectedPins()){
		auto gpio = gpioInterfacePin->getSharedPointer<GpioInterface>();
		connectedDeviceInterfaces.push_back(gpio);
		connectedGpioInterfaces.push_back(gpio);
	}
	
	updateAxisConfiguration();
	
	
	
}


void AxisNode::updateControlMode(){
	
	switch(controlModeParameter->value){
		case ControlMode::POSITION_CONTROL:
			option_SignalAtLowerLimit.enable();
			option_SignalAtLowerAndUpperLimits.enable();
			option_SignalAtOrigin.enable();
			option_LimitAndSlowdownAtLowerAndUpperLimits.disable();
			if(limitSignalTypeParameter->value == option_LimitAndSlowdownAtLowerAndUpperLimits.getInt()){
				limitSignalTypeParameter->overwrite(&option_SignalAtLowerAndUpperLimits);
				updateLimitSignalType();
			}
			break;
		case ControlMode::VELOCITY_CONTROL:
			option_SignalAtLowerLimit.disable();
			option_SignalAtLowerAndUpperLimits.disable();
			option_SignalAtOrigin.disable();
			option_LimitAndSlowdownAtLowerAndUpperLimits.enable();
			if(limitSignalTypeParameter->value != option_NoLimitSignal.getInt() &&
			   limitSignalTypeParameter->value != option_LimitAndSlowdownAtLowerAndUpperLimits.getInt()){
				limitSignalTypeParameter->overwrite(&option_LimitAndSlowdownAtLowerAndUpperLimits);
				updateLimitSignalType();
			}
			break;
		case ControlMode::NO_CONTROL:
			option_SignalAtLowerLimit.disable();
			option_SignalAtLowerAndUpperLimits.disable();
			option_SignalAtOrigin.disable();
			option_LimitAndSlowdownAtLowerAndUpperLimits.disable();
			if(limitSignalTypeParameter->value != option_NoLimitSignal.getInt()){
				limitSignalTypeParameter->overwrite(&option_NoLimitSignal);
				updateLimitSignalType();
			}
			break;
	}
	updateAxisConfiguration();
	
}

void AxisNode::updateLimitSignalType(){
	switch(limitSignalTypeParameter->value){
		case LimitSignalType::NONE:
			//remove
			lowerLimitSignalPin->disconnectAllLinks();
			lowerLimitSignalPin->setVisible(false);
			upperLimitSignalPin->disconnectAllLinks();
			upperLimitSignalPin->setVisible(false);
			lowerSlowdownSignalPin->disconnectAllLinks();
			lowerSlowdownSignalPin->setVisible(false);
			upperSlowdownSignalPin->disconnectAllLinks();
			upperSlowdownSignalPin->setVisible(false);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			option_FindSignalEdge.disable();
			option_FindSignalCenter.disable();
			option_HomingDirectionNegative.disable();
			option_HomingDirectionPositive.disable();
			break;
		case LimitSignalType::SIGNAL_AT_LOWER_LIMIT:
			//add
			lowerLimitSignalPin->setVisible(true);
			//remove
			upperLimitSignalPin->disconnectAllLinks();
			upperLimitSignalPin->setVisible(false);
			lowerSlowdownSignalPin->disconnectAllLinks();
			lowerSlowdownSignalPin->setVisible(false);
			upperSlowdownSignalPin->disconnectAllLinks();
			upperSlowdownSignalPin->setVisible(false);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			
			option_FindSignalEdge.enable();
			option_FindSignalCenter.disable();
			option_HomingDirectionNegative.enable();
			option_HomingDirectionPositive.disable();
			homingDirectionParameter->overwrite(&option_HomingDirectionNegative);
			signalApproachParameter->overwrite(&option_FindSignalEdge);
			break;
		case LimitSignalType::SIGNAL_AT_LOWER_AND_UPPER_LIMITS:
			//add
			lowerLimitSignalPin->setVisible(true);
			upperLimitSignalPin->setVisible(true);
			//remove
			lowerSlowdownSignalPin->disconnectAllLinks();
			lowerSlowdownSignalPin->setVisible(false);
			upperSlowdownSignalPin->disconnectAllLinks();
			upperSlowdownSignalPin->setVisible(false);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			
			option_FindSignalEdge.enable();
			option_FindSignalCenter.disable();
			option_HomingDirectionNegative.enable();
			option_HomingDirectionPositive.enable();
			signalApproachParameter->overwrite(&option_FindSignalEdge);
			break;
		case LimitSignalType::SIGNAL_AT_ORIGIN:
			//add
			referenceSignalPin->setVisible(true);
			//remove
			lowerLimitSignalPin->disconnectAllLinks();
			lowerLimitSignalPin->setVisible(false);
			upperLimitSignalPin->disconnectAllLinks();
			upperLimitSignalPin->setVisible(false);
			lowerSlowdownSignalPin->disconnectAllLinks();
			lowerSlowdownSignalPin->setVisible(false);
			upperSlowdownSignalPin->disconnectAllLinks();
			upperSlowdownSignalPin->setVisible(false);
			
			option_FindSignalEdge.enable();
			option_FindSignalCenter.enable();
			option_HomingDirectionNegative.enable();
			option_HomingDirectionPositive.enable();
			break;
		case LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS:
			//add
			lowerLimitSignalPin->setVisible(true);
			upperLimitSignalPin->setVisible(true);
			lowerSlowdownSignalPin->setVisible(true);
			upperSlowdownSignalPin->setVisible(true);
			//remove
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			
			option_FindSignalEdge.disable();
			option_FindSignalCenter.disable();
			option_HomingDirectionNegative.disable();
			option_HomingDirectionPositive.disable();
			break;
		default:
			break;
	}
	updateAxisConfiguration();
}

void AxisNode::updateAxisConfiguration(){
	auto& config = axisInterface->configuration;

	switch(controlModeParameter->value){
		case ControlMode::VELOCITY_CONTROL:
			config.controlMode = AxisInterface::ControlMode::VELOCITY_CONTROL;
			break;
		case ControlMode::POSITION_CONTROL:
			config.controlMode = AxisInterface::ControlMode::POSITION_CONTROL;
			break;
		default:
			config.controlMode = AxisInterface::ControlMode::NONE;
			break;
	}
	
	config.accelerationLimit = accelerationLimit->value;
	config.decelerationLimit = accelerationLimit->value;
	config.velocityLimit = velocityLimit->value;
	config.lowerPositionLimit = lowerPositionLimit->value + std::abs(lowerPositionLimitClearance->value);
	config.upperPositionLimit = upperPositionLimit->value - std::abs(upperPositionLimitClearance->value);
	
	config.b_supportsPositionFeedback = positionFeedbackMapping != nullptr;
	config.b_supportsVelocityFeedback = velocityFeedbackMapping != nullptr;
	config.b_supportsForceFeedback = false;
	
	bool b_force = false;
	for(auto actuatorMapping : actuatorMappings){
		if(actuatorMapping->actuatorInterface->supportsEffortFeedback()){
			b_force = true;
			break;
		}
	}
	config.b_supportsEffortFeedback = b_force;
	
	config.b_supportsHoming = config.controlMode == AxisInterface::ControlMode::POSITION_CONTROL &&
							limitSignalTypeParameter->value != option_NoLimitSignal.getInt();
}

