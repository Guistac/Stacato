#include <pch.h>

#include "GpioServoActuator.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void GpioServoActuator::initialize(){
	//input pins
	addNodePin(gpioDevicePin);
	addNodePin(readyPin);
	addNodePin(brakePin);
	addNodePin(emergencyStopPin);
	addNodePin(positionFeedbackPin);
	//output pins
	addNodePin(enablePin);
	addNodePin(controlSignalPin);
	addNodePin(servoActuatorPin);
}

void GpioServoActuator::process(){
	
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
	auto gpioDevice = getGpioDevice();
	
	//update input signals
	readyPin->copyConnectedPinValue();
	brakePin->copyConnectedPinValue();
	emergencyStopPin->copyConnectedPinValue();
	
	//update servo actuator data
	servoActuator->b_online = true;
	servoActuator->b_detected = true;
	servoActuator->b_canHardReset = feedbackDevice->canHardReset();
	servoActuator->b_parked = false;
	servoActuator->b_ready = feedbackDevice->isReady() && gpioDevice->isReady() && *readySignal && !*emergencyStopSignal && !*brakeSignal;
	servoActuator->positionRaw_positionUnits = feedbackUnitsToActuatorUnits(feedbackDevice->positionRaw_positionUnits);
	servoActuator->velocity_positionUnitsPerSecond = feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity());;
	servoActuator->b_moving = std::abs(servoActuator->velocity_positionUnitsPerSecond) > 0.0;
	servoActuator->b_emergencyStopActive = *emergencyStopSignal;
	servoActuator->load = 0.0; //load is not supported here
	
	if(servoActuator->getPosition() - servoActuator->getCommand() > maxFollowingError){
		servoActuator->b_enabled = false;
		onDisable();
	}
}

void GpioServoActuator::setVelocityTarget(double target){
	controlMode = ControlMode::VELOCITY_TARGET;
	manualVelocityTarget = target;
}

void GpioServoActuator::onDisable(){
	
}

void GpioServoActuator::onEnable(){
	
}


bool GpioServoActuator::areAllPinsConnected(){
	//inputs
	if(!isPositionFeedbackConnected()) return false;
	if(!isGpioDeviceConnected()) return false;
	if(!readyPin->isConnected()) return false;
	if(!brakePin->isConnected()) return false;
	if(!emergencyStopPin->isConnected()) return false;
	//outputs
	if(!enablePin->isConnected()) return false;
	if(!controlSignalPin->isConnected()) return false;
	if(!servoActuatorPin->isConnected()) return false;
}


void GpioServoActuator::updatePin(std::shared_ptr<NodePin> pin){
	if(pin == servoActuatorPin){
		
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
			*enableSignal = true;
			
			switch(controlMode){
				case ControlMode::EXTERNAL:{
					//get profile position and velocity from actuator pin
					double previousProfilePosition = motionProfile.getPosition();
					double newProfilePosition = servoActuator->getCommand();
					motionProfile.setPosition(newProfilePosition);
					double calculatedProfileVelocity = (newProfilePosition - previousProfilePosition) / profileTimeDelta_seconds;
					motionProfile.setVelocity(calculatedProfileVelocity);
					}break;
				case ControlMode::VELOCITY_TARGET:
					//get profile position from manual velocity generator
					motionProfile.matchVelocity(profileTimeDelta_seconds, manualVelocityTarget, manualAcceleration);
					break;
				default:
					break;
			}
			
			//calculate position error and apply gains to generate control signal
			double realPosition = servoActuator->getPosition();
			double positionError = motionProfile.getPosition() - realPosition;
			//========= CONTROL LOOP ========
			outputVelocity = positionError * proportionalGain + motionProfile.getVelocity() * derivativeGain;
			
		}else{
			*enableSignal = false;
			outputVelocity = 0.0;
			motionProfile.setPosition(servoActuator->getPosition());
			motionProfile.setVelocity(servoActuator->getVelocity());
			motionProfile.setAcceleration(0.0);
		}
		
		//generate control signal output value and assign to output pin
		*controlSignal = actuatorVelocityToControlSignal(outputVelocity);
	}
}


//the gpio servo actuator does the following:
//read all node inputs to evaluate servo actuator state
//specify an output voltage range mapped to a velocity range
//specify velocity and acceleration limits
//specify gain settings for the control loop
//

double GpioServoActuator::getControlSignalLowLimit(){
	if(b_controlSignalIsCenteredOnZero) return -controlSignalRange;
	else return 0.0;
}

double GpioServoActuator::getControlSignalHighLimit(){
	if(b_controlSignalIsCenteredOnZero) return controlSignalRange;
	else return controlSignalRange;
}

double GpioServoActuator::getControlSignalZero(){
	if(b_controlSignalIsCenteredOnZero) return 0.0;
	else return controlSignalRange / 2.0;
}

double GpioServoActuator::controlSignalToActuatorVelocity(double signal){
	if(b_invertControlSignal) return (signal - getControlSignalZero()) / -controlSignalUnitsPerActuatorVelocityUnit;
	return (signal - getControlSignalZero()) / controlSignalUnitsPerActuatorVelocityUnit;
}

double GpioServoActuator::actuatorVelocityToControlSignal(double velocity){
	if(b_invertControlSignal) return getControlSignalZero() - velocity * controlSignalUnitsPerActuatorVelocityUnit;
	return getControlSignalZero() + velocity * controlSignalUnitsPerActuatorVelocityUnit;
}

double GpioServoActuator::getControlSignalLimitVelocity(){
	return std::abs(getControlSignalHighLimit() / controlSignalUnitsPerActuatorVelocityUnit);
}

double GpioServoActuator::feedbackUnitsToActuatorUnits(double feedbackValue){
	return feedbackValue / positionFeedbackUnitsPerActuatorUnit;
}



void GpioServoActuator::sanitizeParameters(){
	controlSignalRange = std::abs(controlSignalRange);
	controlSignalUnitsPerActuatorVelocityUnit = std::abs(controlSignalUnitsPerActuatorVelocityUnit);
	servoActuator->velocityLimit_positionUnitsPerSecond = std::abs(servoActuator->velocityLimit_positionUnitsPerSecond);
	servoActuator->velocityLimit_positionUnitsPerSecond = std::min(getControlSignalLimitVelocity(), servoActuator->velocityLimit_positionUnitsPerSecond);
	servoActuator->accelerationLimit_positionUnitsPerSecondSquared = std::abs(servoActuator->accelerationLimit_positionUnitsPerSecondSquared);
	maxFollowingError = std::abs(maxFollowingError);
}





bool GpioServoActuator::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->InsertNewChildElement("ServoActuator");
	actuatorXML->SetAttribute("PositionUnit", Enumerator::getSaveString(servoActuator->getPositionUnit()));
	actuatorXML->SetAttribute("VelocityLimit", servoActuator->getVelocityLimit());
	actuatorXML->SetAttribute("AccelerationLimit", servoActuator->getAccelerationLimit());
	
	XMLElement* controlSignalXML = xml->InsertNewChildElement("ControlSignal");
	controlSignalXML->SetAttribute("Range", controlSignalRange);
	controlSignalXML->SetAttribute("CenterOnZero", b_controlSignalIsCenteredOnZero);
	controlSignalXML->SetAttribute("Invert", b_invertControlSignal);
	
	XMLElement* conversionRatioXML = xml->InsertNewChildElement("ConversionRatio");
	conversionRatioXML->SetAttribute("ControlSignalUnitsPerActuatorVelocityUnit", controlSignalUnitsPerActuatorVelocityUnit);
	conversionRatioXML->SetAttribute("PositionFeedbackUnitsPerActuatorPositionUnit", positionFeedbackUnitsPerActuatorUnit);
	
	XMLElement* controllerXML = xml->InsertNewChildElement("Controller");
	controllerXML->SetAttribute("ProportionalGain", proportionalGain);
	controllerXML->SetAttribute("DerivativeGain", derivativeGain);
	controllerXML->SetAttribute("MaxFollowingError", maxFollowingError);
	
	return true;
}

bool GpioServoActuator::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->FirstChildElement("ServoActuator");
	if(actuatorXML == nullptr) return Logger::warn("Could not find Servo Actuator attribute");
	const char* actuatorUnitString;
	actuatorXML->QueryStringAttribute("PositionUnit", &actuatorUnitString);
	if(!Enumerator::isValidSaveName<PositionUnit>(actuatorUnitString)) return Logger::warn("Could not identify servo actuator position unit");
	servoActuator->positionUnit = Enumerator::getEnumeratorFromSaveString<PositionUnit>(actuatorUnitString);
	if(actuatorXML->QueryDoubleAttribute("VelocityLimit", &servoActuator->velocityLimit_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not find actuator velocity limit Attribute");
	if(actuatorXML->QueryDoubleAttribute("AccelerationLimit", &servoActuator->accelerationLimit_positionUnitsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not find acceleration limit Attribute");
	
	XMLElement* controlSignalXML = xml->FirstChildElement("ControlSignal");
	if(controlSignalXML == nullptr) return Logger::warn("Could not find control signal attribute");
	if(controlSignalXML->QueryDoubleAttribute("Range", &controlSignalRange) != XML_SUCCESS) return Logger::warn("Could not find control signal range attribute");
	if(controlSignalXML->QueryBoolAttribute("CenterOnZero", &b_controlSignalIsCenteredOnZero) != XML_SUCCESS) return Logger::warn("Could not find center on zero attribute");
	if(controlSignalXML->QueryBoolAttribute("Invert", &b_invertControlSignal) != XML_SUCCESS) return Logger::warn("Could not find invert attribute");
	
	XMLElement* conversionRatioXML = xml->FirstChildElement("ConversionRatio");
	if(conversionRatioXML == nullptr) return Logger::warn("Could not find Conversion Ratio attribute");
	if(conversionRatioXML->QueryDoubleAttribute("ControlSignalUnitsPerActuatorVelocityUnit", &controlSignalUnitsPerActuatorVelocityUnit) != XML_SUCCESS) return Logger::warn("Could not find signal unit conversion attribute");
	if(conversionRatioXML->QueryDoubleAttribute("PositionFeedbackUnitsPerActuatorPositionUnit", &positionFeedbackUnitsPerActuatorUnit) != XML_SUCCESS) return Logger::warn("Could not find feedback unit conversion attribute");
	
	XMLElement* controllerXML = xml->FirstChildElement("Controller");
	if(controllerXML == nullptr) return Logger::warn("Could not find Controller attribute");
	if(controllerXML->QueryDoubleAttribute("ProportionalGain", &proportionalGain) != XML_SUCCESS) return Logger::warn("Could not find proportional gain attribute");
	if(controllerXML->QueryDoubleAttribute("DerivativeGain", &derivativeGain) != XML_SUCCESS) return Logger::warn("Could not find derivative gain attribute");
	if(controllerXML->QueryDoubleAttribute("MaxFollowingError", &maxFollowingError) != XML_SUCCESS) return Logger::warn("Could not find max following error attribute");
	 
	return true;
}


