#include <pch.h>
#include "AxisNode.h"

#include "Motion/Interfaces.h"









void AxisNode::onConstruction(){
	Node::onConstruction();
	
	setName("Axis");
	
	axisInterface = std::make_shared<AxisInterface>();
	
	lowerLimitSignal = std::make_shared<bool>(false);
	upperLimitSignal = std::make_shared<bool>(false);
	referenceSignal = std::make_shared<bool>(false);
	surveillanceResetSignal = std::make_shared<bool>(false);
	lowerSlowdownSignal = std::make_shared<bool>(false);
	upperSlowdownSignal = std::make_shared<bool>(false);
	
	brakeControlSignal = std::make_shared<bool>(false);
	surveillanceValidSignal = std::make_shared<bool>(false);
	
	actuatorPin = NodePin::createInstance(NodePin::DataType::ACTUATOR_INTERFACE,
											NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
											"Actuator", "Actuator",
											NodePin::Flags::AcceptMultipleInputs);
	feedbackPin = NodePin::createInstance(NodePin::DataType::MOTIONFEEDBACK_INTERFACE,
											NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
											"Feedback", "Feedback",
											NodePin::Flags::AcceptMultipleInputs);
	gpioPin = NodePin::createInstance(NodePin::DataType::GPIO_INTERFACE,
										NodePin::Direction::NODE_INPUT,
										"GPIO", "GPIO",
										NodePin::Flags::AcceptMultipleInputs);
	lowerLimitSignalPin = NodePin::createInstance(lowerLimitSignal, NodePin::Direction::NODE_INPUT,
													"Lower Limit Signal", "LowerLimitSignal");
	upperLimitSignalPin = NodePin::createInstance(upperLimitSignal, NodePin::Direction::NODE_INPUT,
													"Upper Limit Signal", "UpperLimitSignal");
	lowerSlowdownSignalPin = NodePin::createInstance(lowerSlowdownSignal, NodePin::Direction::NODE_INPUT,
													   "Lower Slowdown Signal", "LowerSlowdownSignal");
	upperSlowdownSignalPin = NodePin::createInstance(upperSlowdownSignal, NodePin::Direction::NODE_INPUT,
													   "Upper Slowdown Signal", "UpperSlowdownSignal");
	referenceSignalPin = NodePin::createInstance(referenceSignal, NodePin::Direction::NODE_INPUT,
												   "Reference Signal", "ReferenceSignal");
	surveillanceResetSignalPin = NodePin::createInstance(surveillanceResetSignal, NodePin::Direction::NODE_INPUT,
														   "Surveillance Fault Reset", "SurveillanceFaultReset");
	
	axisPin = NodePin::createInstance(axisInterface, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL,
										"Axis", "Axis");
	brakeControlSignalPin = NodePin::createInstance(brakeControlSignal, NodePin::Direction::NODE_OUTPUT,
													  "Brake Control Signal", "BrakeControlSignal");
	surveillanceValidSignalPin = NodePin::createInstance(surveillanceValidSignal, NodePin::Direction::NODE_OUTPUT,
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
	
	
	
	
	
	movementTypeParameter = OptionParameter::make2(option_movementType_Linear, {
														&option_movementType_Linear,
														&option_movementType_Angular
													}, "Movement Type", "MovementType");
	movementTypeParameter->addEditCallback([this](){ updateMovementType(); });
	
	positionUnitParameter = OptionParameter::make2(option_positionUnit_Meter, {
														&option_positionUnit_Millimeter,
														&option_positionUnit_Centimeter,
														&option_positionUnit_Meter,
														&option_positionUnit_Degree,
														&option_positionUnit_Radian,
														&option_positionUnit_Revolution
													}, "Position Unit", "PositionUnit");
	positionUnitParameter->addEditCallback([this](){ updatePositionUnit(); });
	
	controlModeParameter = OptionParameter::make2(option_controlMode_None, {
		&option_controlMode_Position,
		&option_controlMode_Velocity,
		&option_controlMode_None
	}, "Axis Control Mode", "AxisControlMode");
	controlModeParameter->addEditCallback([this](){ updateControlMode(); });
	
	maxEnableTimeSeconds = NumberParameter<double>::make(0.2, "Max enable time (seconds)", "MaxEnableTime");
	maxEnableTimeSeconds->setUnit(Units::Time::Second);
	
	positionLoop_velocityFeedForward = NumberParameter<double>::make(100.0, "Position loop velocity feed forward (PvFF)", "PositionLoopVelocityFeedForward");
	positionLoop_velocityFeedForward->setUnit(Units::Fraction::Percent);
	positionLoop_proportionalGain = NumberParameter<double>::make(0.0, "Position loop proportional gain (PKp)", "PositionLoopProportionalGain");
	positionLoop_maxError = NumberParameter<double>::make(0.0, "Position loop max error", "PositionLoopMaxError");
	positionLoop_minError = NumberParameter<double>::make(0.0, "Position loop min error", "PositionLoopMinError");
	
	velocityLoop_maxError = NumberParameter<double>::make(0.0, "Velocity loop max error", "VelocityLoopMaxError");
	velocityLoop_maxError->setSuffix("/s");
	limitSlowdownVelocity = NumberParameter<double>::make(0.0, "Limit Slowdown Velocity", "LimitSlowdownVelocity");
	limitSlowdownVelocity->setSuffix("/s");
	
	enableLowerPositionLimit = BooleanParameter::make(false, "Enable Lower Position Limit", "EnableLowerPositionLimit");
	enableUpperPositionLimit = BooleanParameter::make(false, "Enable Upper Position Limit", "EnableUpperPositionLimit");
	limitPositionToFeedbackWorkingRange = BooleanParameter::make(true, "Limit position to feedback working range", "LimitPositionToFeedbackWorkingRange");
	lowerPositionLimit = 			NumberParameter<double>::make(0.0, "Lower Position Limit", "LowerPositionLimit");
	upperPositionLimit = 			NumberParameter<double>::make(0.0, "Upper Position Limit", "UpperPositionLimit");
	lowerPositionLimitClearance = 	NumberParameter<double>::make(0.0, "Lower Position Limit Clearance", "LowerPositionLimitClearance");
	upperPositionLimitClearance = 	NumberParameter<double>::make(0.0, "Upper Position Limit Clearance", "UpperPositionLimitClearance");
	velocityLimit = 				NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit");
	velocityLimit->setSuffix("/s");
	accelerationLimit = 			NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit");
	accelerationLimit->setSuffix("/s\xc2\xb2");
	
	auto updateInterfaceCallback = [this](){updateAxisConfiguration();};
	enableLowerPositionLimit->addEditCallback(updateInterfaceCallback);
	enableUpperPositionLimit->addEditCallback(updateInterfaceCallback);
	limitPositionToFeedbackWorkingRange->addEditCallback(updateInterfaceCallback);
	lowerPositionLimit->addEditCallback(updateInterfaceCallback);
	upperPositionLimit->addEditCallback(updateInterfaceCallback);
	lowerPositionLimitClearance->addEditCallback(updateInterfaceCallback);
	upperPositionLimitClearance->addEditCallback(updateInterfaceCallback);
	velocityLimit->addEditCallback(updateInterfaceCallback);
	accelerationLimit->addEditCallback(updateInterfaceCallback);
	
	
	
	limitSignalTypeParameter = OptionParameter::make2(option_LimitSignalType_NoLimitSignal, {
															&option_LimitSignalType_NoLimitSignal,
															&option_LimitSignalType_SignalAtLowerLimit,
															&option_LimitSignalType_SignalAtLowerAndUpperLimits,
															&option_LimitSignalType_SignalAtOrigin,
															&option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits
														}, "Limit Signal Type", "LimitSignalType");
	
	homingDirectionParameter = 	OptionParameter::make2(option_HomingDirection_Negative, {
															&option_HomingDirection_Negative,
															&option_HomingDirection_Positive
														}, "Homing direction", "HomingDirection");
	
	signalApproachParameter = 	OptionParameter::make2(option_SignalApproachMethod_FindSignalEdge, {
															&option_SignalApproachMethod_FindSignalEdge,
															&option_SignalApproachMethod_FindSignalCenter
														}, "Signal approach method", "SignalApproachMethod");
	
	homingVelocityCoarse = 		NumberParameter<double>::make(0.0, "Homing velocity coarse", "HomingVelocityCoarse");
	homingVelocityCoarse->setSuffix("/s");
	homingVelocityFine = 		NumberParameter<double>::make(0.0, "Homing velocity fine", "HomingVelocityFine");
	homingVelocityFine->setSuffix("/s");
	maxHomingDistanceCoarse = 	NumberParameter<double>::make(0.0, "Max homing distance coarse", "MaxHomingDistanceCoarse");
	maxHomingDistanceFine = 	NumberParameter<double>::make(0.0, "Max homing distance fine", "MaxHomingDistanceFine");
	limitSignalTypeParameter->addEditCallback([this](){ updateLimitSignalType(); });
	homingDirectionParameter->addEditCallback([this](){ updateLimitSignalType(); });
	signalApproachParameter->addEditCallback([this](){ updateLimitSignalType(); });

	updateMovementType();
	updateControlMode();
	updateLimitSignalType();
	updateAxisConfiguration();
}

void AxisNode::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == actuatorPin || pin == feedbackPin || pin == gpioPin){
		updateConnectedModules();
	}
}
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
		velocityFeedbackMapping->feedbackUnitsPerAxisUnit->save(vfbXML);
	}
	
	XMLElement* actuatorMappingsXML = xml->InsertNewChildElement("ActuatorMappings");
	for(auto actuatorMapping : actuatorMappings){
		XMLElement* actXML = actuatorMappingsXML->InsertNewChildElement("ActuatorMapping");
		actXML->SetAttribute("InterfacePinID", actuatorMapping->interfacePinID);
		actuatorMapping->actuatorUnitsPerAxisUnits->save(actXML);
		actuatorMapping->controlModeParameter->save(actXML);
	}
	
	movementTypeParameter->save(xml);
	positionUnitParameter->save(xml);
	limitSignalTypeParameter->save(xml);
	controlModeParameter->save(xml);
	
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
	
	homingDirectionParameter->save(xml);
	signalApproachParameter->save(xml);
	homingVelocityCoarse->save(xml);
	homingVelocityFine->save(xml);
	maxHomingDistanceCoarse->save(xml);
	maxHomingDistanceFine->save(xml);
	
	return true;
}

bool AxisNode::load(tinyxml2::XMLElement* xml){
	
	bool success = true;
	
	success &= controlModeParameter->load(xml);
	success &= limitSignalTypeParameter->load(xml);
	success &= movementTypeParameter->load(xml);
	success &= positionUnitParameter->load(xml);
	
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
	
	success &= homingDirectionParameter->load(xml);
	success &= signalApproachParameter->load(xml);
	success &= homingVelocityCoarse->load(xml);
	success &= homingVelocityFine->load(xml);
	success &= maxHomingDistanceCoarse->load(xml);
	success &= maxHomingDistanceFine->load(xml);
	
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
				auto thisAxisNode = std::static_pointer_cast<AxisNode>(shared_from_this());
				positionFeedbackMapping = std::make_shared<FeedbackMapping>(fbPin, thisAxisNode);
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
				auto thisAxisNode = std::static_pointer_cast<AxisNode>(shared_from_this());
				velocityFeedbackMapping = std::make_shared<FeedbackMapping>(fbPin, thisAxisNode);
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
	
	updateControlMode();
	updateMovementType();
	

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
			auto thisAxisNode = std::static_pointer_cast<AxisNode>(shared_from_this());
			auto newMapping = std::make_shared<ActuatorMapping>(actuatorPin, thisAxisNode);
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
			controlMode = ControlMode::POSITION_CONTROL;
			option_LimitSignalType_SignalAtLowerLimit.enable();
			option_LimitSignalType_SignalAtLowerAndUpperLimits.enable();
			option_LimitSignalType_SignalAtOrigin.enable();
			option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits.disable();
			if(limitSignalTypeParameter->value == LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS){
				limitSignalTypeParameter->overwrite(&option_LimitSignalType_SignalAtLowerAndUpperLimits);
				updateLimitSignalType();
			}
			break;
		case ControlMode::VELOCITY_CONTROL:
			controlMode = ControlMode::VELOCITY_CONTROL;
			option_LimitSignalType_SignalAtLowerLimit.disable();
			option_LimitSignalType_SignalAtLowerAndUpperLimits.disable();
			option_LimitSignalType_SignalAtOrigin.disable();
			option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits.enable();
			if(limitSignalTypeParameter->value != LimitSignalType::NONE &&
			   limitSignalTypeParameter->value != LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS){
				limitSignalTypeParameter->overwrite(&option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits);
				updateLimitSignalType();
			}
			break;
		case ControlMode::NO_CONTROL:
			controlMode = ControlMode::NO_CONTROL;
			option_LimitSignalType_SignalAtLowerLimit.disable();
			option_LimitSignalType_SignalAtLowerAndUpperLimits.disable();
			option_LimitSignalType_SignalAtOrigin.disable();
			option_LimitSignalType_LimitAndSlowdownAtLowerAndUpperLimits.disable();
			if(limitSignalTypeParameter->value != LimitSignalType::NONE){
				limitSignalTypeParameter->overwrite(&option_LimitSignalType_NoLimitSignal);
				updateLimitSignalType();
			}
			break;
	}
	updateLimitSignalType();
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
			option_SignalApproachMethod_FindSignalEdge.disable();
			option_SignalApproachMethod_FindSignalCenter.disable();
			option_HomingDirection_Negative.disable();
			option_HomingDirection_Positive.disable();
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
			
			option_SignalApproachMethod_FindSignalEdge.enable();
			option_SignalApproachMethod_FindSignalCenter.disable();
			option_HomingDirection_Negative.enable();
			option_HomingDirection_Positive.disable();
			homingDirectionParameter->overwrite(&option_HomingDirection_Negative);
			signalApproachParameter->overwrite(&option_SignalApproachMethod_FindSignalEdge);
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
			
			option_SignalApproachMethod_FindSignalEdge.enable();
			option_SignalApproachMethod_FindSignalCenter.disable();
			option_HomingDirection_Negative.enable();
			option_HomingDirection_Positive.enable();
			signalApproachParameter->overwrite(&option_SignalApproachMethod_FindSignalEdge);
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
			
			option_SignalApproachMethod_FindSignalEdge.enable();
			option_SignalApproachMethod_FindSignalCenter.enable();
			option_HomingDirection_Negative.enable();
			option_HomingDirection_Positive.enable();
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
			
			option_SignalApproachMethod_FindSignalEdge.disable();
			option_SignalApproachMethod_FindSignalCenter.disable();
			option_HomingDirection_Negative.disable();
			option_HomingDirection_Positive.disable();
			break;
		default:
			break;
	}
	
	limitSignalType = (LimitSignalType)limitSignalTypeParameter->value;
	homingDirection = (HomingDirection)homingDirectionParameter->value;
	signalApproachMethod = (SignalApproachMethod)signalApproachParameter->value;
	
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
	
	double lowestActuatorVelocityLimit = std::numeric_limits<double>::infinity();
	double lowestActuatorAccelerationLimit = std::numeric_limits<double>::infinity();
	for(auto actuatorMapping : actuatorMappings){
		auto actuator = actuatorMapping->actuatorInterface;
		double thisActuatorVelocityLimit = actuator->getVelocityLimit() / actuatorMapping->actuatorUnitsPerAxisUnits->value;
		double thisActuatorAccelerationLimit = actuator->getAccelerationLimit()/ actuatorMapping->actuatorUnitsPerAxisUnits->value;
		lowestActuatorVelocityLimit = std::min(lowestActuatorVelocityLimit, thisActuatorVelocityLimit);
		lowestActuatorAccelerationLimit = std::min(lowestActuatorAccelerationLimit, thisActuatorAccelerationLimit);
	}
	actuatorVelocityLimit = lowestActuatorVelocityLimit;
	actuatorAccelerationLimit = lowestActuatorAccelerationLimit;
	
	if(actuatorVelocityLimit < velocityLimit->value) velocityLimit->overwriteWithHistory(actuatorVelocityLimit);
	if(actuatorAccelerationLimit < accelerationLimit->value) accelerationLimit->overwriteWithHistory(actuatorAccelerationLimit);
	
	config.accelerationLimit = accelerationLimit->value;
	config.decelerationLimit = accelerationLimit->value;
	config.velocityLimit = velocityLimit->value;
	
	if(positionFeedbackMapping){
		auto feedback = positionFeedbackMapping->feedbackInterface;
		
		feedbackLowerPositionLimit = feedback->getPositionLowerWorkingRangeBound() / positionFeedbackMapping->feedbackUnitsPerAxisUnit->value;
		feedbackUpperPositionLimit = feedback->getPositionUpperWorkingRangeBound() / positionFeedbackMapping->feedbackUnitsPerAxisUnit->value;
		
		if(enableLowerPositionLimit->value)
			lowerPositionLimitWithoutClearance = std::clamp(lowerPositionLimit->value, feedbackLowerPositionLimit, feedbackUpperPositionLimit);
		else
			lowerPositionLimitWithoutClearance = feedbackLowerPositionLimit;
		if(enableUpperPositionLimit->value)
			upperPositionLimitWithoutClearance = std::clamp(upperPositionLimit->value, feedbackLowerPositionLimit, feedbackUpperPositionLimit);
		else
			upperPositionLimitWithoutClearance = feedbackUpperPositionLimit;
	}else{
		lowerPositionLimitWithoutClearance = -std::numeric_limits<double>::infinity();
		upperPositionLimitWithoutClearance = std::numeric_limits<double>::infinity();
	}
		
	config.lowerPositionLimit = lowerPositionLimitWithoutClearance + std::abs(lowerPositionLimitClearance->value);
	config.upperPositionLimit = upperPositionLimitWithoutClearance - std::abs(upperPositionLimitClearance->value);
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
							limitSignalTypeParameter->value != LimitSignalType::NONE &&
							limitSignalTypeParameter->value != LimitSignalType::LIMIT_AND_SLOWDOWN_SIGNALS_AT_LOWER_AND_UPPER_LIMITS;
	
	
	axisPin->updateConnectedPins();
}


void AxisNode::updateMovementType(){
	switch(movementTypeParameter->value){
		case MovementType::LINEAR:
			movementType = MovementType::LINEAR;
			option_positionUnit_Millimeter.enable();
			option_positionUnit_Centimeter.enable();
			option_positionUnit_Meter.enable();
			option_positionUnit_Degree.disable();
			option_positionUnit_Radian.disable();
			option_positionUnit_Revolution.disable();
			if(axisInterface->getPositionUnit()->unitType != Units::Type::LINEAR_DISTANCE){
				positionUnitParameter->overwrite(&option_positionUnit_Meter);
			}
			break;
		case MovementType::ANGULAR:
			movementType = MovementType::ANGULAR;
			option_positionUnit_Millimeter.disable();
			option_positionUnit_Centimeter.disable();
			option_positionUnit_Meter.disable();
			option_positionUnit_Degree.enable();
			option_positionUnit_Radian.enable();
			option_positionUnit_Revolution.enable();
			if(axisInterface->getPositionUnit()->unitType != Units::Type::ANGULAR_DISTANCE){
				positionUnitParameter->overwrite(&option_positionUnit_Degree);
			}
			break;
	}
	updatePositionUnit();
}

void AxisNode::updatePositionUnit(){
	
	Unit newPositionUnit = Units::None::None;
	int unit = positionUnitParameter->value;
	if(unit == option_positionUnit_Millimeter.getInt()) 		newPositionUnit = Units::LinearDistance::Millimeter;
	else if(unit == option_positionUnit_Centimeter.getInt()) 	newPositionUnit = Units::LinearDistance::Centimeter;
	else if(unit == option_positionUnit_Meter.getInt()) 		newPositionUnit = Units::LinearDistance::Meter;
	else if(unit == option_positionUnit_Degree.getInt()) 		newPositionUnit = Units::AngularDistance::Degree;
	else if(unit == option_positionUnit_Radian.getInt()) 		newPositionUnit = Units::AngularDistance::Radian;
	else if(unit == option_positionUnit_Revolution.getInt()) 	newPositionUnit = Units::AngularDistance::Revolution;
	
	axisInterface->configuration.positionUnit = newPositionUnit;
	
	std::vector<std::shared_ptr<BaseNumberParameter>> positionUnitParameters = {
		positionLoop_maxError,
		positionLoop_minError,
		limitSlowdownVelocity,
		velocityLoop_maxError,
		lowerPositionLimit,
		upperPositionLimit,
		lowerPositionLimitClearance,
		upperPositionLimitClearance,
		velocityLimit,
		accelerationLimit,
		homingVelocityCoarse,
		homingVelocityFine,
		maxHomingDistanceCoarse,
		maxHomingDistanceFine
	};
	for(auto unitParameter : positionUnitParameters){
		unitParameter->setUnit(newPositionUnit);
	}
	axisPin->updateConnectedPins();
}


std::string AxisNode::getHomingStepString(){
	switch(homingStep){
		case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
			return "Searching low limit (coarse)";
		case HomingStep::FOUND_LOW_LIMIT_COARSE:
			return "Found low limit (coarse)";
		case HomingStep::SEARCHING_LOW_LIMIT_FINE:
			return "Searching low limit (fine)";
		case HomingStep::FOUND_LOW_LIMIT:
			return "Found low limit";
		case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
			return "Searching high limit (coarse)";
		case HomingStep::FOUND_HIGH_LIMIT_COARSE:
			return "Found high limit (coarse)";
		case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
			return "Searching high limit (fine)";
		case HomingStep::FOUND_HIGH_LIMIT:
			return "found high limit";
		case HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_COARSE:
			return "Searching origin upper edge (coarse)";
		case HomingStep::FOUND_ORIGIN_UPPER_EDGE_COARSE:
			return "Found origin upper edge (coarse)";
		case HomingStep::SEARCHING_ORIGIN_UPPER_EDGE_FINE:
			return "Searching origin upper edge (fine)";
		case HomingStep::FOUND_ORIGIN_UPPER_EDGE:
			return "Found origin upper edge";
		case HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_COARSE:
			return "Searching origin lower edge (coarse)";
		case HomingStep::FOUND_ORIGIN_LOWER_EDGE_COARSE:
			return "Found origin lower edge (coarse)";
		case HomingStep::SEARCHING_ORIGIN_LOWER_EDGE_FINE:
			return "Searching origin lower edge (fine)";
		case HomingStep::FOUND_ORIGIN_LOWER_EDGE:
			return "Found origin lower edge";
		case HomingStep::MOVING_TO_ORIGIN_CENTER:
			return "Moving to origin center";
		case HomingStep::NOT_STARTED:
			return "Not started";
		case HomingStep::RESETTING_POSITION_FEEDBACK:
			return "Resetting position feedback";
		case HomingStep::FINISHING:
			return "Finishing...";
		case HomingStep::FINISHED:
			return "Finished";
		case HomingStep::FAILED:
			return "Failed";
	}
}
