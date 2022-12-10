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

void ActuatorToServoActuator::inputProcess(){
	
	//update time no matter what
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//abort if not all connections are made
	if(!areAllPinsConnected()) {
		servoActuator->state = MotionState::OFFLINE;
		return;
	}
	
	//get device references
	auto feedbackDevice = getPositionFeedbackDevice();
	auto actuatorDevice = getActuatorDevice();
	
	//handle actuator state change requests enabling
	if(servoActuator->b_enable){
		servoActuator->b_enable = false;
		enable();
	}
	if(servoActuator->b_disable){
		servoActuator->b_disable = false;
		disable();
	}
	
	//update servo actuator data
	servoActuator->positionUnit = actuatorDevice->positionUnit;
	servoActuator->b_online = true;
	servoActuator->b_detected = true;
	servoActuator->b_parked = false;
	servoActuator->b_ready = feedbackDevice->isReady() && actuatorDevice->isReady();
	servoActuator->positionRaw_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->positionRaw_positionUnits);
	servoActuator->velocity_positionUnitsPerSecond = feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity());;
	servoActuator->b_moving = feedbackDevice->getVelocity() != 0.0;
	servoActuator->b_emergencyStopActive = actuatorDevice->isEmergencyStopActive();
	servoActuator->load = 0.0; //load is not supported here
	servoActuator->b_brakesActive = actuatorDevice->areBrakesActive();
	servoActuator->rangeMin_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->rangeMin_positionUnits);
	servoActuator->rangeMax_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->rangeMax_positionUnits);
	servoActuator->velocityLimit_positionUnitsPerSecond = actuatorDevice->velocityLimit_positionUnitsPerSecond;
	servoActuator->minVelocity_positionUnitsPerSecond = actuatorDevice->minVelocity_positionUnitsPerSecond;
	servoActuator->accelerationLimit_positionUnitsPerSecondSquared = actuatorDevice->accelerationLimit_positionUnitsPerSecondSquared;

	//propagate the request backwards to the encoder
	if(servoActuator->b_doHardReset){
		servoActuator->b_doHardReset = false;
		feedbackDevice->b_doHardReset = true;
	}
	servoActuator->b_canHardReset = feedbackDevice->canHardReset();
	servoActuator->b_isHardResetting = feedbackDevice->b_isHardResetting;
	
	if(!servoActuator->isEnabled()){
		motionProfile.setPosition(feedbackUnitsToActuatorUnits(feedbackDevice->getPosition()));
		motionProfile.setVelocity(feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity()));
		return;
	}
	
	//if the servo actuator pin is connected, all manual controls are disabled
	//else the node does its own processing of the control loop
	if(servoActuatorPin->isConnected()) controlMode = ControlMode::EXTERNAL;
	else controlLoop();
}

void ActuatorToServoActuator::outputProcess(){
	Logger::critical("output process not defined for actuator to servo actuator");
	abort();
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
		auto feedbackDevice = getPositionFeedbackDevice();
		//TODO: position command raw or profile position ??
		realPosition = feedbackUnitsToActuatorUnits(feedbackDevice->getPosition());
		realVelocity = feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity());
		double followingError = motionProfile.getPosition() - realPosition;
		if(std::abs(followingError) > servoActuator->maxfollowingError) {
			disable();
			Logger::critical("Servo Actuator '{}' disabled : max following error exceeded", getName());
		}
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
		
		if(targetVelocity == 0.0 && std::abs(positionError) < errorCorrectionTreshold){
			//don't do position correction if we are below a certain error threshold and the target velocity is zero
			outputVelocity = 0.0;
		}else{
			outputVelocity = positionError * positionLoopProportionalGain + targetVelocity * velocityLoopProportionalGain;
		}
		
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
			while(system_clock::now() - start < milliseconds(500)){
				if(actuator->isEnabled()){
					servoActuator->b_enabled = true;
					Logger::info("Enabled Actuator to Servo Actuator '{}'", getName());
					onEnable();
					return;
				}
				std::this_thread::sleep_for(milliseconds(10));
			}
			servoActuator->b_enabled = false;
			onDisable();
			actuator->disable();
			Logger::warn("Could not enable Actuator to Servo Actuator '{}'", getName());
		});
		actuatorEnabler.detach();
		
		onEnable();
	}
}

void ActuatorToServoActuator::disable(){
	if(servoActuator->isEnabled()){
		servoActuator->b_enabled = false;
		onDisable();
		Logger::error("DISABLING ACTUATOR");
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

void ActuatorToServoActuator::onEnable(){
	motionProfile.setPosition(feedbackUnitsToActuatorUnits(getPositionFeedbackDevice()->getPosition()));
}


bool ActuatorToServoActuator::areAllPinsConnected(){
	//inputs
	if(!isPositionFeedbackConnected()) return false;
	if(!isActuatorConnected()) return false;
	return true;
}


//needs to be called by controlling node to exexute control loop
void ActuatorToServoActuator::onPinUpdate(std::shared_ptr<NodePin> pin){
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
	servoActuator->maxfollowingError = std::abs(servoActuator->maxfollowingError);
}


bool ActuatorToServoActuator::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	 
	XMLElement* conversionRatioXML = xml->InsertNewChildElement("ConversionRatio");
	conversionRatioXML->SetAttribute("PositionFeedbackUnitsPerActuatorPositionUnit", positionFeedbackUnitsPerActuatorUnit);
	
	XMLElement* controllerXML = xml->InsertNewChildElement("Controller");
	controllerXML->SetAttribute("PositionLoopProportionalGain", positionLoopProportionalGain);
	controllerXML->SetAttribute("VelocityLoopProportionalGain", velocityLoopProportionalGain);
	controllerXML->SetAttribute("VelocityLoopIntegralGain", velocityLoopIntegralGain);
	controllerXML->SetAttribute("MaxPositionFollowingError", servoActuator->maxfollowingError);
	controllerXML->SetAttribute("ErrorCorrectionTreshold", errorCorrectionTreshold);
	
	XMLElement* manualControlsXML = xml->InsertNewChildElement("ManualControls");
	manualControlsXML->SetAttribute("Acceleration", manualAcceleration);
	
	return true;
}

bool ActuatorToServoActuator::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* conversionRatioXML = xml->FirstChildElement("ConversionRatio");
	if(conversionRatioXML == nullptr) return Logger::warn("Could not find Conversion Ratio attribute");
	if(conversionRatioXML->QueryDoubleAttribute("PositionFeedbackUnitsPerActuatorPositionUnit", &positionFeedbackUnitsPerActuatorUnit) != XML_SUCCESS) return Logger::warn("Could not find feedback unit conversion attribute");
	
	XMLElement* controllerXML = xml->FirstChildElement("Controller");
	if(controllerXML == nullptr) return Logger::warn("Could not find Controller attribute");
	if(controllerXML->QueryDoubleAttribute("PositionLoopProportionalGain", &positionLoopProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find position loop proportional gain attribute");
	if(controllerXML->QueryDoubleAttribute("VelocityLoopProportionalGain", &velocityLoopProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find velocity loop proportional gain attribute");
	if(controllerXML->QueryDoubleAttribute("VelocityLoopIntegralGain", &velocityLoopIntegralGain) != XML_SUCCESS) return Logger::warn("Could not find velocity loop integral gain attribute");
	if(controllerXML->QueryDoubleAttribute("MaxPositionFollowingError", &servoActuator->maxfollowingError) != XML_SUCCESS) return Logger::warn("Could not find max following error attribute");
	if(controllerXML->QueryDoubleAttribute("ErrorCorrectionTreshold", &errorCorrectionTreshold) != XML_SUCCESS) return Logger::warn("Could not find error correction threshold attribute");
	XMLElement* manualControlsXML = xml->FirstChildElement("ManualControls");
	if(manualControlsXML == nullptr) return Logger::warn("Could not find Manual Controls attribute");
	if(manualControlsXML->QueryDoubleAttribute("Acceleration", &manualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find manual acceleration target");
	
	return true;
}


