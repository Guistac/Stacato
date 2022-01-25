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
	servoActuator->rangeMin_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->rangeMin_positionUnits);
	servoActuator->rangeMax_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->rangeMax_positionUnits);
	
	//TODO: position command raw or profile position ??
	realPosition = feedbackUnitsToActuatorUnits(feedbackDevice->getPosition());
	realVelocity = feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity());
	double followingError = motionProfile.getPosition() - realPosition;
	if(std::abs(followingError) > maxPositionFollowingError){
		disable();
	}
	
	//if the servi actuator pin is connected, all manual controls are disabled
	//else the node does its own processing of the control loop
	if(servoActuatorPin->isConnected()) controlMode = ControlMode::EXTERNAL;
	else controlLoop();
}

void ActuatorToServoActuator::controlLoop(){
	//handle actuator state change requests enabling
	if(servoActuator->b_setEnabled){
		servoActuator->b_setEnabled = false;
		enable();
	}
	if(servoActuator->b_setDisabled){
		servoActuator->b_setDisabled = false;
		disable();
	}
	
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
		targetPosition = motionProfile.getPosition();
		targetVelocity = motionProfile.getVelocity();
		positionError = targetPosition - realPosition;
		velocityError = targetVelocity - realVelocity;
		//========= CONTROL LOOP ========
		//TODO: this needs some real work
		
		outputVelocity = positionError * positionLoopProportionalGain + targetVelocity * velocityLoopProportionalGain;
		
	}else{
		outputVelocity = 0.0;
		motionProfile.setPosition(servoActuator->getPosition());
		motionProfile.setVelocity(servoActuator->getVelocity());
		motionProfile.setAcceleration(0.0);
	}
	
	//send velocity command to actuator
	auto actuatorDevice = getActuatorDevice();
	actuatorDevice->setVelocityCommand(outputVelocity);
	actuatorPin->updateConnectedPins();
	
	positionTargetHistory.addPoint(glm::vec2(profileTime_seconds, 	targetPosition));
	positionHistory.addPoint(glm::vec2(profileTime_seconds, 		realPosition));
	velocityTargetHistory.addPoint(glm::vec2(profileTime_seconds, 	targetVelocity));
	velocityHistory.addPoint(glm::vec2(profileTime_seconds, 		realVelocity));
	positionErrorHistory.addPoint(glm::vec2(profileTime_seconds, 	positionError));
	velocityErrorHistory.addPoint(glm::vec2(profileTime_seconds, 	velocityError));
}

void ActuatorToServoActuator::enable(){
	if(servoActuator->isReady()) {
		
		std::thread actuatorEnabler([this](){
			using namespace std::chrono;
			auto actuator = getActuatorDevice();
			actuator->enable();
			system_clock::time_point start = system_clock::now();
			while(system_clock::now() - start < milliseconds(100)){
				if(actuator->isEnabled()){
					servoActuator->b_enabled = true;
					return;
				}
			}
			servoActuator->b_enabled = false;
			actuator->disable();
		});
		actuatorEnabler.detach();
		
		onEnable();
	}
}

void ActuatorToServoActuator::disable(){
	if(servoActuator->isEnabled()){
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

void ActuatorToServoActuator::onDisable(){}

void ActuatorToServoActuator::onEnable(){}


bool ActuatorToServoActuator::areAllPinsConnected(){
	//inputs
	if(!isPositionFeedbackConnected()) return false;
	if(!isActuatorConnected()) return false;
	return true;
}


//needs to be called by controlling node to exexute control loop
void ActuatorToServoActuator::updatePin(std::shared_ptr<NodePin> pin){
	if(pin == servoActuatorPin){
		controlLoop();
	}
}










void ActuatorToServoActuator::sanitizeParameters(){
	if(isActuatorConnected()){
		auto actuatorDevice = getActuatorDevice();
		servoActuator->velocityLimit_positionUnitsPerSecond = std::min(std::abs(servoActuator->velocityLimit_positionUnitsPerSecond), actuatorDevice->getVelocityLimit());
		servoActuator->accelerationLimit_positionUnitsPerSecondSquared = std::min(std::abs(servoActuator->accelerationLimit_positionUnitsPerSecondSquared), actuatorDevice->getAccelerationLimit());
		manualAcceleration = std::min(std::abs(manualAcceleration), servoActuator->accelerationLimit_positionUnitsPerSecondSquared);
	}
	maxPositionFollowingError = std::abs(maxPositionFollowingError);
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
	controllerXML->SetAttribute("PositionLoopProportionalGain", positionLoopProportionalGain);
	controllerXML->SetAttribute("VelocityLoopProportionalGain", velocityLoopProportionalGain);
	controllerXML->SetAttribute("VelocityLoopIntegralGain", velocityLoopIntegralGain);
	controllerXML->SetAttribute("MaxPositionFollowingError", maxPositionFollowingError);
	
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
	if(controllerXML->QueryDoubleAttribute("PositionLoopProportionalGain", &positionLoopProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find position loop proportional gain attribute");
	if(controllerXML->QueryDoubleAttribute("VelocityLoopProportionalGain", &velocityLoopProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find velocity loop proportional gain attribute");
	if(controllerXML->QueryDoubleAttribute("VelocityLoopIntegralGain", &velocityLoopIntegralGain) != XML_SUCCESS) return Logger::warn("Could not find velocity loop integral gain attribute");
	if(controllerXML->QueryDoubleAttribute("MaxPositionFollowingError", &maxPositionFollowingError) != XML_SUCCESS) return Logger::warn("Could not find max following error attribute");
	 
	XMLElement* manualControlsXML = xml->FirstChildElement("ManualControls");
	if(manualControlsXML == nullptr) return Logger::warn("Could not find Manual Controls attribute");
	if(manualControlsXML->QueryDoubleAttribute("Acceleration", &manualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find manual acceleration target");
	
	return true;
}


