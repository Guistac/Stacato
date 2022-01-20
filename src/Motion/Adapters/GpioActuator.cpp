#include <pch.h>

#include "GpioActuator.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"

void GpioActuator::initialize(){
	//input pins
	addNodePin(gpioDevicePin);
	addNodePin(readyPin);
	addNodePin(brakePin);
	addNodePin(emergencyStopPin);
	//output pins
	addNodePin(enablePin);
	addNodePin(controlSignalPin);
	addNodePin(actuatorPin);
}

void GpioActuator::process(){
	
	//update time no matter what
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//abort if not all connections are made
	if(!areAllPinsConnected()) {
		actuator->b_online = false;
		actuator->b_detected = false;
		actuator->b_ready = false;
		return;
	}
	
	//if the actuator pin is enabled, all manual controls are disabled
	if(actuatorPin->isConnected()) controlMode = ControlMode::EXTERNAL;
	
	//get device reference
	auto gpioDevice = getGpioDevice();
	
	//update input signals
	readyPin->copyConnectedPinValue();
	brakePin->copyConnectedPinValue();
	emergencyStopPin->copyConnectedPinValue();
	
	//update servo actuator data
	actuator->b_online = true;
	actuator->b_detected = true;
	actuator->b_parked = false;
	actuator->b_ready = gpioDevice->isReady() && *readySignal && !*emergencyStopSignal && !*brakeSignal;
	
	//actuator->velocity_positionUnitsPerSecond = feedbackUnitsToActuatorUnits(feedbackDevice->getVelocity());;
	//actuator->b_moving = std::abs(actuator->velocity_positionUnitsPerSecond) > 0.0;
	actuator->b_emergencyStopActive = *emergencyStopSignal;
	actuator->load = 0.0; //load is not supported here
	
}

void GpioActuator::updatePin(std::shared_ptr<NodePin> pin){
	if(pin == actuatorPin){
		//generate enable signal & control voltage outputs
	}
}


bool GpioActuator::areAllPinsConnected(){
	if(!isGpioDeviceConnected()) return false;
	if(!readyPin->isConnected()) return false;
	if(!brakePin->isConnected()) return false;
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

double GpioActuator::getControlSignalLimitVelocity(){
	return std::abs(getControlSignalHighLimit() / controlSignalUnitsPerActuatorVelocityUnit);
}



bool GpioActuator::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->InsertNewChildElement("Actuator");
	actuatorXML->SetAttribute("PositionUnit", Enumerator::getSaveString(actuator->getPositionUnit()));
	actuatorXML->SetAttribute("VelocityLimit", actuator->getVelocityLimit());
	actuatorXML->SetAttribute("AccelerationLimit", actuator->getAccelerationLimit());
	
	XMLElement* controlSignalXML = xml->InsertNewChildElement("ControlSignal");
	controlSignalXML->SetAttribute("Range", controlSignalRange);
	controlSignalXML->SetAttribute("CenterOnZero", b_controlSignalIsCenteredOnZero);
	controlSignalXML->SetAttribute("Invert", b_invertControlSignal);
	
	XMLElement* conversionRatioXML = xml->InsertNewChildElement("ConversionRatio");
	conversionRatioXML->SetAttribute("ControlSignalUnitsPerActuatorVelocityUnit", controlSignalUnitsPerActuatorVelocityUnit);
		
	return true;
}

bool GpioActuator::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* actuatorXML = xml->FirstChildElement("Actuator");
	if(actuatorXML == nullptr) return Logger::warn("Could not find Servo Actuator attribute");
	const char* actuatorUnitString;
	actuatorXML->QueryStringAttribute("PositionUnit", &actuatorUnitString);
	if(!Enumerator::isValidSaveName<PositionUnit>(actuatorUnitString)) return Logger::warn("Could not identify servo actuator position unit");
	actuator->positionUnit = Enumerator::getEnumeratorFromSaveString<PositionUnit>(actuatorUnitString);
	if(actuatorXML->QueryDoubleAttribute("VelocityLimit", &actuator->velocityLimit_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not find actuator velocity limit Attribute");
	if(actuatorXML->QueryDoubleAttribute("AccelerationLimit", &actuator->accelerationLimit_positionUnitsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not find acceleration limit Attribute");
	
	
	XMLElement* controlSignalXML = xml->FirstChildElement("ControlSignal");
	if(controlSignalXML == nullptr) return Logger::warn("Could not find control signal attribute");
	if(controlSignalXML->QueryDoubleAttribute("Range", &controlSignalRange) != XML_SUCCESS) return Logger::warn("Could not find control signal range attribute");
	if(controlSignalXML->QueryBoolAttribute("CenterOnZero", &b_controlSignalIsCenteredOnZero) != XML_SUCCESS) return Logger::warn("Could not find center on zero attribute");
	if(controlSignalXML->QueryBoolAttribute("Invert", &b_invertControlSignal) != XML_SUCCESS) return Logger::warn("Could not find invert attribute");
	
	
	XMLElement* conversionRatioXML = xml->FirstChildElement("ConversionRatio");
	if(conversionRatioXML == nullptr) return Logger::warn("Could not find Conversion Ratio attribute");
	if(conversionRatioXML->QueryDoubleAttribute("ControlSignalUnitsPerActuatorVelocityUnit", &controlSignalUnitsPerActuatorVelocityUnit) != XML_SUCCESS) return Logger::warn("Could not find signal unit conversion attribute");
	
	return true;
}



