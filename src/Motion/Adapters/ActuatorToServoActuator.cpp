#include <pch.h>

#include "ActuatorToServoActuator.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void ActuatorToServoActuator::initialize(){
	//input pins
	addNodePin(actuatorPin);
	addNodePin(positionFeedbackPin);
	//output pins
	addNodePin(servoActuatorPin);
}

void ActuatorToServoActuator::process(){
	
	//update time no matter what
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//abort if not all connections are made
	if(!areAllPinsConnected()) {
		servoActuator->b_online = false;
		servoActuator->b_detected = false;
		servoActuator->b_ready = false;
		return;
	}
	
	//if the actuator pin is enabled, all manual controls are disabled
	if(servoActuatorPin->isConnected()) controlMode = ControlMode::EXTERNAL;
	
	//get device references
	auto feedbackDevice = getPositionFeedbackDevice();
	auto actuatorDevice = getActuatorDevice();
	
	//update servo actuator data
	servoActuator->positionUnit = actuatorDevice->positionUnit;
	servoActuator->b_online = true;
	servoActuator->b_detected = true;
	servoActuator->b_canHardReset = feedbackDevice->canHardReset();
	servoActuator->b_parked = false;
	servoActuator->b_ready = feedbackDevice->isReady() && actuatorDevice->isReady();
	servoActuator->positionRaw_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->positionRaw_positionUnits);
	servoActuator->velocity_positionUnitsPerSecond = feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity());;
	servoActuator->b_moving = std::abs(servoActuator->velocity_positionUnitsPerSecond) > 0.0;
	servoActuator->b_emergencyStopActive = actuatorDevice->isEmergencyStopActive();
	servoActuator->load = 0.0; //load is not supported here
	servoActuator->b_brakesActive = actuatorDevice->areBrakesActive();
	servoActuator->rangeMin_positionUnits = feedbackDevice->rangeMin_positionUnits;
	servoActuator->rangeMax_positionUnits = feedbackDevice->rangeMax_positionUnits;
	
	//TODO: position command raw or profile position ??
	if(servoActuator->getPosition() - servoActuator->getPositionCommandRaw() > maxFollowingError){
		servoActuator->b_enabled = false;
		onDisable();
	}
}

void ActuatorToServoActuator::setVelocityTarget(double target){
	controlMode = ControlMode::VELOCITY_TARGET;
	manualVelocityTarget = target;
}

void ActuatorToServoActuator::fastStop(){
	controlMode = ControlMode::FAST_STOP;
}

void ActuatorToServoActuator::moveToPositionInTime(double targetPosition, double targetTime){
	targetPosition = std::min(targetPosition, servoActuator->getMaxPosition());
	targetPosition = std::max(targetPosition, servoActuator->getMinPosition());
	bool success = motionProfile.moveToPositionInTime(profileTime_seconds,
													  targetPosition,
													  targetTime,
													  manualAcceleration,
													  servoActuator->velocityLimit_positionUnitsPerSecond);
	if(success) controlMode = ControlMode::POSITION_TARGET;
	else setVelocityTarget(0.0);
}

void ActuatorToServoActuator::movetoPositionWithVelocity(double targetPosition, double targetVelocity){
	targetPosition = std::min(targetPosition, servoActuator->getMaxPosition());
	targetPosition = std::max(targetPosition, servoActuator->getMinPosition());
	bool success = motionProfile.moveToPositionWithVelocity(profileTime_seconds,
															targetPosition,
															targetVelocity,
															manualAcceleration);
	if(success) controlMode = ControlMode::POSITION_TARGET;
	else setVelocityTarget(0.0);
}

void ActuatorToServoActuator::controlLoop(){
	//handle actuator enabling
	if(servoActuator->b_setEnabled){
		servoActuator->b_setEnabled = false;
		if(servoActuator->b_ready) {
			servoActuator->b_enabled = true;
			onEnable();
		}
	}
	
	//get output velocity
	double outputVelocity;
	
	if(servoActuator->isEnabled()){
		switch(controlMode){
			case ControlMode::FAST_STOP:
				motionProfile.stop(profileTimeDelta_seconds, servoActuator->getAccelerationLimit());
				break;
			case ControlMode::EXTERNAL:
				motionProfile.setPosition(servoActuator->getPositionCommandRaw());
				motionProfile.setVelocity(servoActuator->getVelocityCommand());
				break;
			case ControlMode::VELOCITY_TARGET:
				motionProfile.matchVelocity(profileTimeDelta_seconds, manualVelocityTarget, manualAcceleration);
				break;
			case ControlMode::POSITION_TARGET:
				motionProfile.updateInterpolation(profileTime_seconds);
				if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
			default:
				break;
		}
		
		//calculate position error and apply gains to generate control signal
		double realPosition = servoActuator->getPosition();
		double positionError = motionProfile.getPosition() - realPosition;
		//========= CONTROL LOOP ========
		outputVelocity = positionError * proportionalGain + motionProfile.getVelocity() * derivativeGain;
		
	}else{
		outputVelocity = 0.0;
		motionProfile.setPosition(servoActuator->getPosition());
		motionProfile.setVelocity(servoActuator->getVelocity());
		motionProfile.setAcceleration(0.0);
	}
	
	auto actuatorDevice = getActuatorDevice();
	actuatorDevice->setVelocityCommand(outputVelocity);
	actuatorPin->updateConnectedPins();
}

void ActuatorToServoActuator::onDisable(){}

void ActuatorToServoActuator::onEnable(){}


bool ActuatorToServoActuator::areAllPinsConnected(){
	//inputs
	if(!isPositionFeedbackConnected()) return false;
	if(!isActuatorConnected()) return false;
	//outputs
	if(!servoActuatorPin->isConnected()) return false;
}


void ActuatorToServoActuator::updatePin(std::shared_ptr<NodePin> pin){
	if(pin == servoActuatorPin){
		controlLoop();
	}
}










void ActuatorToServoActuator::sanitizeParameters(){
	auto actuatorDevice = getActuatorDevice();
	servoActuator->velocityLimit_positionUnitsPerSecond = std::min(std::abs(servoActuator->velocityLimit_positionUnitsPerSecond), actuatorDevice->getVelocityLimit());
	servoActuator->accelerationLimit_positionUnitsPerSecondSquared = std::min(std::abs(servoActuator->accelerationLimit_positionUnitsPerSecondSquared), actuatorDevice->getAccelerationLimit());
	maxFollowingError = std::abs(maxFollowingError);
	manualAcceleration = std::min(std::abs(manualAcceleration), servoActuator->accelerationLimit_positionUnitsPerSecondSquared);
}


bool ActuatorToServoActuator::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->InsertNewChildElement("ServoActuator");
	actuatorXML->SetAttribute("PositionUnit", Enumerator::getSaveString(servoActuator->getPositionUnit()));
	actuatorXML->SetAttribute("VelocityLimit", servoActuator->getVelocityLimit());
	actuatorXML->SetAttribute("AccelerationLimit", servoActuator->getAccelerationLimit());
	
	XMLElement* conversionRatioXML = xml->InsertNewChildElement("ConversionRatio");
	conversionRatioXML->SetAttribute("PositionFeedbackUnitsPerActuatorPositionUnit", positionFeedbackUnitsPerActuatorUnit);
	
	XMLElement* controllerXML = xml->InsertNewChildElement("Controller");
	controllerXML->SetAttribute("ProportionalGain", proportionalGain);
	controllerXML->SetAttribute("DerivativeGain", derivativeGain);
	controllerXML->SetAttribute("MaxFollowingError", maxFollowingError);
	
	XMLElement* manualControlsXML = xml->InsertNewChildElement("ManualControls");
	manualControlsXML->SetAttribute("Acceleration", manualAcceleration);
	
	return true;
}

bool ActuatorToServoActuator::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->FirstChildElement("ServoActuator");
	if(actuatorXML == nullptr) return Logger::warn("Could not find Servo Actuator attribute");
	const char* actuatorUnitString;
	actuatorXML->QueryStringAttribute("PositionUnit", &actuatorUnitString);
	if(!Enumerator::isValidSaveName<PositionUnit>(actuatorUnitString)) return Logger::warn("Could not identify servo actuator position unit");
	servoActuator->positionUnit = Enumerator::getEnumeratorFromSaveString<PositionUnit>(actuatorUnitString);
	if(actuatorXML->QueryDoubleAttribute("VelocityLimit", &servoActuator->velocityLimit_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not find actuator velocity limit Attribute");
	if(actuatorXML->QueryDoubleAttribute("AccelerationLimit", &servoActuator->accelerationLimit_positionUnitsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not find acceleration limit Attribute");
	
	XMLElement* conversionRatioXML = xml->FirstChildElement("ConversionRatio");
	if(conversionRatioXML == nullptr) return Logger::warn("Could not find Conversion Ratio attribute");
	if(conversionRatioXML->QueryDoubleAttribute("PositionFeedbackUnitsPerActuatorPositionUnit", &positionFeedbackUnitsPerActuatorUnit) != XML_SUCCESS) return Logger::warn("Could not find feedback unit conversion attribute");
	
	XMLElement* controllerXML = xml->FirstChildElement("Controller");
	if(controllerXML == nullptr) return Logger::warn("Could not find Controller attribute");
	if(controllerXML->QueryDoubleAttribute("ProportionalGain", &proportionalGain) != XML_SUCCESS) return Logger::warn("Could not find proportional gain attribute");
	if(controllerXML->QueryDoubleAttribute("DerivativeGain", &derivativeGain) != XML_SUCCESS) return Logger::warn("Could not find derivative gain attribute");
	if(controllerXML->QueryDoubleAttribute("MaxFollowingError", &maxFollowingError) != XML_SUCCESS) return Logger::warn("Could not find max following error attribute");
	 
	XMLElement* manualControlsXML = xml->FirstChildElement("ManualControls");
	if(manualControlsXML == nullptr) return Logger::warn("Could not find Manual Controls attribute");
	if(manualControlsXML->QueryDoubleAttribute("Acceleration", &manualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find manual acceleration target");
	
	return true;
}


