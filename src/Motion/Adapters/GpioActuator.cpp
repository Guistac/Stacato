#include <pch.h>

#include "GpioActuator.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"

void GpioActuator::initialize(){
	auto thisGpioActuator = std::static_pointer_cast<GpioActuator>(shared_from_this());
	actuator = std::make_shared<Actuator>(thisGpioActuator);
	actuatorPin->assignData(actuator);
	
	//input pins
	addNodePin(gpioDevicePin);
	addNodePin(emergencyStopPin);
	addNodePin(readyPin);
	//output pins
	addNodePin(enablePin);
	addNodePin(controlSignalPin);
	addNodePin(actuatorPin);
}

void GpioActuator::inputProcess(){
	
	//update time no matter what
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//abort if not all connections are made
	if(!areAllPinsConnected()) {
		actuator->b_wrongConnections = true;
		return;
	}else actuator->b_wrongConnections = false;
		
	//update input signals
	readyPin->copyConnectedPinValue();
	emergencyStopPin->copyConnectedPinValue();
	
	//handle actuator enabling
	if(actuator->b_enable){
		actuator->b_disable = false;
		enable();
	}
	if(actuator->b_disable){
		actuator->b_disable = false;
		disable();
	}
		
	//disable actuator conditions
	if(!*readySignal && actuator->isEnabled()) disable();
	if(*emergencyStopSignal && actuator->isEnabled()) disable();
	
	if(isActuatorPinConnected()) controlMode = ControlMode::EXTERNAL;
	else controlLoop();
}

void GpioActuator::outputProcess(){
	Logger::critical("Output process not defined for gpio actuator");
	abort();
}

void GpioActuator::controlLoop(){
	
	//get output velocity
	double outputVelocity;
	
	if(actuator->isEnabled()){
		switch(controlMode){
			case ControlMode::FAST_STOP:
				motionProfile.stop(profileTimeDelta_seconds, actuator->getAccelerationLimit());
				break;
			case ControlMode::EXTERNAL:
				motionProfile.setVelocity(actuator->velocityCommand);
				motionProfile.setAcceleration(actuator->accelerationCommand);
				break;
			case ControlMode::VELOCITY_TARGET:
				motionProfile.matchVelocity(profileTimeDelta_seconds, manualVelocityTarget, manualAcceleration);
				break;
			default: break;
		}
		outputVelocity = motionProfile.getVelocity();
	}else{
		outputVelocity = 0.0;
		motionProfile.setVelocity(0.0);
		motionProfile.setAcceleration(0.0);
	}
	
	//generate control signal output value and assign to output pin
	//cut off velocity below minimum value
	//if(std::abs(motionProfile.getVelocity()) < actuator->getMinVelocity()) *controlSignal = actuatorVelocityToControlSignal(0.0);
	/*else*/ *controlSignal = actuatorVelocityToControlSignal(outputVelocity);
}

//needs to be called by controlling node to execute control loop
void GpioActuator::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == actuatorPin){
		controlLoop();
	}
}

void GpioActuator::enable(){
	if(actuator->isReady()) {
		*enableSignal = true;
		actuator->b_enabled = true;
		onEnable();
	}
}

void GpioActuator::disable(){
	if(actuator->isEnabled()){
		*enableSignal = false;
		actuator->b_enabled = false;
		onDisable();
	}
}


void GpioActuator::setVelocityTarget(double velocityTarget){
	controlMode = ControlMode::VELOCITY_TARGET;
	manualVelocityTarget = velocityTarget;
}

void GpioActuator::fastStop(){
	controlMode = ControlMode::FAST_STOP;
}

void GpioActuator::onEnable(){}
void GpioActuator::onDisable(){}

bool GpioActuator::areAllPinsConnected(){
	if(!isGpioDeviceConnected()) return false;
	if(!readyPin->isConnected()) return false;
	if(!emergencyStopPin->isConnected()) return false;
	if(!enablePin->isConnected()) return false;
	if(!controlSignalPin->isConnected()) return false;
	return true;
}


double GpioActuator::getControlSignalLowLimit(){
	if(b_controlSignalIsCenteredOnZero) return -controlSignalRange;
	else return 0.0;
}

double GpioActuator::getControlSignalHighLimit(){
	if(b_controlSignalIsCenteredOnZero) return controlSignalRange;
	else return controlSignalRange;
}

double GpioActuator::getControlSignalZero(){
	if(b_controlSignalIsCenteredOnZero) return 0.0;
	else return controlSignalRange / 2.0;
}

double GpioActuator::controlSignalToActuatorVelocity(double signal){
	if(b_invertControlSignal) return (signal - getControlSignalZero()) / -controlSignalUnitsPerActuatorVelocityUnit;
	return (signal - getControlSignalZero()) / controlSignalUnitsPerActuatorVelocityUnit;
}

double GpioActuator::actuatorVelocityToControlSignal(double velocity){
	if(b_invertControlSignal) return getControlSignalZero() - velocity * controlSignalUnitsPerActuatorVelocityUnit;
	return getControlSignalZero() + velocity * controlSignalUnitsPerActuatorVelocityUnit;
}

double GpioActuator::getControlSignalLimitedVelocity(){
	return std::abs(getControlSignalHighLimit() / controlSignalUnitsPerActuatorVelocityUnit);
}


void GpioActuator::sanitizeParameters(){
	controlSignalRange = std::abs(controlSignalRange);
	controlSignalUnitsPerActuatorVelocityUnit = std::abs(controlSignalUnitsPerActuatorVelocityUnit);
	positionFeedbackUnitsPerActuatorUnit = std::abs(positionFeedbackUnitsPerActuatorUnit);
	actuator->velocityLimit_positionUnitsPerSecond = std::min(std::abs(actuator->velocityLimit_positionUnitsPerSecond), getControlSignalLimitedVelocity());
	actuator->minVelocity_positionUnitsPerSecond = std::min(std::abs(actuator->minVelocity_positionUnitsPerSecond), actuator->velocityLimit_positionUnitsPerSecond);
	actuator->accelerationLimit_positionUnitsPerSecondSquared = std::abs(actuator->accelerationLimit_positionUnitsPerSecondSquared);
	manualAcceleration = std::min(std::abs(manualAcceleration), actuator->getAccelerationLimit());
}


bool GpioActuator::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->InsertNewChildElement("Actuator");
	actuatorXML->SetAttribute("PositionUnit", actuator->getPositionUnit()->saveString);
	actuatorXML->SetAttribute("VelocityLimit", actuator->getVelocityLimit());
	actuatorXML->SetAttribute("MinVelocity", actuator->getMinVelocity());
	actuatorXML->SetAttribute("AccelerationLimit", actuator->getAccelerationLimit());
	
	XMLElement* controlSignalXML = xml->InsertNewChildElement("ControlSignal");
	controlSignalXML->SetAttribute("Range", controlSignalRange);
	controlSignalXML->SetAttribute("CenterOnZero", b_controlSignalIsCenteredOnZero);
	controlSignalXML->SetAttribute("Invert", b_invertControlSignal);
	
	XMLElement* conversionRatioXML = xml->InsertNewChildElement("ConversionRatio");
	conversionRatioXML->SetAttribute("ControlSignalUnitsPerActuatorVelocityUnit", controlSignalUnitsPerActuatorVelocityUnit);
		
	XMLElement* manualControlsXML = xml->InsertNewChildElement("ManualControls");
	manualControlsXML->SetAttribute("Acceleration", manualAcceleration);
	
	return true;
}

bool GpioActuator::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->FirstChildElement("Actuator");
	if(actuatorXML == nullptr) return Logger::warn("Could not find Servo Actuator attribute");
	const char* actuatorUnitString;
	actuatorXML->QueryStringAttribute("PositionUnit", &actuatorUnitString);
	if(!Units::isValidSaveString(actuatorUnitString)) return Logger::warn("Could not identify servo actuator position unit");
	actuator->positionUnit = Units::fromSaveString(actuatorUnitString);
	if(actuatorXML->QueryDoubleAttribute("VelocityLimit", &actuator->velocityLimit_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not find actuator velocity limit Attribute");
	if(actuatorXML->QueryDoubleAttribute("MinVelocity", &actuator->minVelocity_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not find actuator minimum velocity Attribute");
	if(actuatorXML->QueryDoubleAttribute("AccelerationLimit", &actuator->accelerationLimit_positionUnitsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not find acceleration limit Attribute");
	
	XMLElement* controlSignalXML = xml->FirstChildElement("ControlSignal");
	if(controlSignalXML == nullptr) return Logger::warn("Could not find control signal attribute");
	if(controlSignalXML->QueryDoubleAttribute("Range", &controlSignalRange) != XML_SUCCESS) return Logger::warn("Could not find control signal range attribute");
	if(controlSignalXML->QueryBoolAttribute("CenterOnZero", &b_controlSignalIsCenteredOnZero) != XML_SUCCESS) return Logger::warn("Could not find center on zero attribute");
	if(controlSignalXML->QueryBoolAttribute("Invert", &b_invertControlSignal) != XML_SUCCESS) return Logger::warn("Could not find invert attribute");
	
	XMLElement* conversionRatioXML = xml->FirstChildElement("ConversionRatio");
	if(conversionRatioXML == nullptr) return Logger::warn("Could not find Conversion Ratio attribute");
	if(conversionRatioXML->QueryDoubleAttribute("ControlSignalUnitsPerActuatorVelocityUnit", &controlSignalUnitsPerActuatorVelocityUnit) != XML_SUCCESS) return Logger::warn("Could not find signal unit conversion attribute");
	
	XMLElement* manualControlsXML = xml->FirstChildElement("ManualControls");
	if(manualControlsXML == nullptr) return Logger::warn("Could not find Manual Controls attribute");
	if(manualControlsXML->QueryDoubleAttribute("Acceleration", &manualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find manual acceleration target");
	
	return true;
}



